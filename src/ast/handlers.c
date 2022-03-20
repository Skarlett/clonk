#include "expr.h"
#include "utils.h"
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>


enum onk_lexicon_t grp_dbg_sym(enum Group_t type)
{
  switch (type) {
    case ListT: return onk_list_group_token;
    case MapT: return onk_map_group_token;
    case onk_code_group_tokenT: return onk_code_group_token;
    case TupleT: return onk_tuple_group_token;
    default: return ONK_UNDEFINED_TOKEN;
  };
}


/*
  returns the amount of arguments it pops from the stack
*/
int8_t argc_map(struct onk_token_t *tok)
{

  if (_onk_is_group(tok->type))
    return tok->end;
  
  if (onk_is_tok_operator(tok->type))
    return 2;

  switch (tok->type) {
    case DefSign: return 1;
    case onk_defbody_op_token: return 2;
    case onk_ifcond_op_token: return 1;
    case onk_ifbody_op_token: return 2;
    case onk_apply_op_token: return 2;
    case ONK_NOT_TOKEN: return 1;
    case BitNot: return 1;
    case onk_idx_op_token: return 4;
    case ONK_IMPORT_TOKEN: return 1;
    default: return 0;
  }
}

/* no special algorithms, just an equality test */
void determine_return_ty(struct Expr *bin) {
  if (bin->inner.bin.rhs->datatype == bin->inner.bin.lhs->datatype)
    bin->inner.bin.returns = bin->inner.bin.rhs->datatype;
  else
    bin->inner.bin.returns = DT_UndefT;
}

void mk_null(struct Expr *ex) {
  ex->type = NopExprT;
  ex->datatype = DT_NullT;
}

/*
  integers are placed directly into the output.
*/
int8_t mk_int(
  struct Expr *ex,
  const struct onk_token_t *int_tok,
  const char * src_code
){
  char * _; 
  ex->type = LiteralExprT;
  ex->datatype = DT_IntT;
  memcpy(&ex->origin, int_tok, sizeof(struct onk_token_t));
  errno = 0;

  ex->inner.value.literal.integer =
    str_to_isize(src_code + int_tok->start, &_, 10);
  
  assert(errno != 0);

  return 0;
}

/*
  symbols are placed directly into the output.
*/
int8_t mk_symbol(
  struct Expr *ex,
  struct onk_token_t *word_tok,
  const char * src_code
){
  uint8_t size = word_tok->end - word_tok->start;
   
  memcpy(&ex->origin, word_tok, sizeof(struct onk_token_t));
  ex->type = SymExprT;
  ex->datatype = DT_UndefT;
  ex->inner.symbol = calloc(1, size+1);
  
  assert(ex->inner.symbol != 0);
  
  memcpy(ex->inner.symbol,
    src_code + word_tok->start,
    size + 1
  );
  
  ex->inner.symbol[size] = 0;
  return 0;
}

/*
  strings are placed directly into the output.
*/
int8_t mk_str(
  struct Expr *ex,
  struct onk_token_t *str_token,
  const char * src_code
){
  char * str;
  uint16_t size = str_token->end - str_token->start;

  ex->type = LiteralExprT;
  ex->datatype = DT_StringT;
  memcpy(&ex->origin, str_token, sizeof(struct onk_token_t));

  str = malloc(size+1);
  assert(str != 0);
  
  ex->inner.value.literal.string = str;

  memcpy(
    ex->inner.value.literal.string,
    &src_code[str_token->start],
    size
  );

  ex->inner.value.literal.string[size] = 0;
  return 0;
}

enum Group_t get_group_t_from_tok(enum onk_lexicon_t tok) {
  switch (tok) {
    case onk_tuple_group_token: return MapT;
    case onk_list_group_token: return ListT;
    case onk_map_group_token: return MapT;
    case SetGroup: return SetT;
    case onk_code_group_token: return onk_code_group_tokenT;
    default: return GroupTUndef;
  }
}

int8_t mk_group(
  struct Expr *ex,
  struct PostfixStageState *state,
  struct onk_token_t *group_tok,
  uint16_t argc
){
  struct Expr **buf = 0;
  /* in Grouping tokens, the `end` parameter determines group size */

  ex->type = LiteralExprT;
  ex->datatype = DT_CollectionT;
  ex->inner.grp.type = get_group_ty(group_tok);
  ex->inner.grp.ptr = 0;
  ex->inner.grp.length = argc;

  memcpy(&ex->origin, group_tok, sizeof(struct onk_token_t));  
  
  if (argc > 0 && state->stack_ctr > argc)
  {
    /* allocate a space for the pointers */
    buf = calloc(argc, sizeof(void *));
    
    if (!buf)
      return -1;
    
    ex->inner.grp.ptr = buf;
    ex->inner.grp.length = argc;

    for (uint16_t j = 0; argc > j; j++) {
      ex->inner.grp.ptr[j] = state->stack[state->stack_ctr - 1];
      //state->stack_ctr -= 1;
    }
  }

  return 0;
}


int8_t mk_binop(
  struct Expr *ex,
  struct onk_token_t *operator,
  struct PostfixStageState *state
){ 
  assert(state->stack_ctr > 0);
  
  memcpy(&ex->origin, operator, sizeof(struct onk_token_t));
  
  ex->type = BinaryExprT;
  ex->inner.bin.lhs = 
    state->stack[state->stack_ctr - 1];

  ex->inner.bin.rhs = 
    state->stack[state->stack_ctr - 2];
  
  ex->inner.bin.op = operation_from_token(operator->type);
  if (ex->inner.bin.op == UndefinedOp)
    return -1;
  
  determine_return_ty(ex);
  // state->stack_ctr -= 2;

  return 0;
}

bool is_unary(enum onk_lexicon_t tok)
{
  return tok == ONK_TILDE_TOKEN || tok == ONK_NOT_TOKEN;
}

int8_t mk_unary(
  struct Expr *ex,
  struct PostfixStageState *state,
  struct onk_token_t *ophead
){
  assert(state->stack_ctr > 0);
  memcpy(&ex->origin, ophead, sizeof(struct onk_token_t));
  
  ex->inner.unary.op = operation_from_token(ophead->type);
  ex->inner.unary.operand = state->stack[state->stack_ctr - 1];
  //state->stack_ctr -= 1;

  return 0;
}

/* 
 * 
 * TODO fix:
 *    Currently `x[1]` produces the same as `x[1:]`
 *    by placing `ONK_NULL_TOKEN` as the `end` attr
 *
 */
int8_t mk_idx_access(
  struct Expr *ex,
  struct PostfixStageState *state
){  
  ex->type = FnCallExprT;
  ex->datatype = 0;
    
  ex->inner.idx.start   = state->stack[state->stack_ctr - 1];
  ex->inner.idx.end     = state->stack[state->stack_ctr - 2];
  ex->inner.idx.skip    = state->stack[state->stack_ctr - 3];
  ex->inner.idx.operand = state->stack[state->stack_ctr - 4];

  //state->stack_ctr -= 4;
  return 0;
}

/* 
 * onk_apply_op_token operations pop N+1 arguments from the stack where 
 * N is derived from `struct onk_parse_group_t`'s `delmiter_ctr` + 1.
 *
 * `delimiter_ctr + 1` representing the total number of arguments.
 * `N + 1` representing the total arguments and the function name
 *
 * Noteworthy: foo()() will produce a top level APPLY expression, 
 * with a nested APPLY expression
 * 
 * src: name(args, ...) 
 * dbg: <name> <args> ... onk_apply_op_token
 *
 */
int8_t mk_fncall(
  struct Expr *ex,
  struct PostfixStageState *state
){
  /* assumes current token is APPLY */
  /*
    grab the previous expression (onk_tuple_group_token parsing-object), 
    and use it's length to determine the amount of arguments
    in the function call.
  */
  struct Expr *args = state->stack[state->stack_ctr - 1];
  struct Expr *caller = state->stack[state->stack_ctr - 2];
  
  /* assert its a group and of tuple type*/
  assert(args->datatype == GroupT && args->inner.grp.type == TupleT);
  
  
  ex->type = FnCallExprT;
  ex->datatype = 0;
  ex->inner.fncall.args = args;

  ex->inner.fncall.func_name = "anonymous";
  ex->inner.fncall.caller = caller;
  ex->inner.fncall.returns = DT_UndefT;
  
  /* add function name */
  if (caller->type == SymExprT)
    ex->inner.fncall.func_name = caller->inner.symbol;

  return 0;
}

int8_t mk_if_cond(
  struct Expr *ex,
  struct PostfixStageState *state
){
  struct Expr *cond = state->stack[state->stack_ctr - 1];

  assert(state->stack_ctr > 0);  
  // state->stack_ctr -= 1;
  
  // todo
  //memcpy(&ex->origin, op_head(state), sizeof(struct onk_token_t));

  ex->type = IfExprT;
  ex->inner.cond.cond = cond;

  return 0;
}

int8_t mk_if_body(struct PostfixStageState *state)
{
  struct Expr *body, *cond = 0;
  assert(state->stack_ctr > 1);
  
  //memcpy(&ex->origin, , sizeof(struct onk_token_t));
  cond = state->stack[state->stack_ctr - 2];
  body = state->stack[state->stack_ctr - 1];
  
  if (cond->type != IfExprT)
    return -1;
  
  cond->inner.cond.body = body;
  state->stack_ctr -= 1;

  return 0;
}

int8_t mk_else_body(struct PostfixStageState *state)
{
  struct Expr 
    *body = state->stack[state->stack_ctr - 1],
    *cond = state->stack[state->stack_ctr - 2];
  
  assert(state->stack_ctr > 1 && cond->type != IfExprT);
  
  cond->inner.cond.else_body = body;
  state->stack_ctr -= 1;

  return 0;
}

int8_t mk_return(
  struct Expr *ex,
  struct PostfixStageState *state,
  struct onk_token_t *ret_tok
){
  struct Expr *inner;
  assert(state->stack_ctr > 0);
  
  // todo: fix
  memcpy(&ex->origin, ret_tok, sizeof(struct onk_token_t));
  
  ex->inner.ret.body = state->stack[state->stack_ctr - 1];
  ex->type = ReturnExprT; 
  
  // state->stack_ctr -= 1;
  return 0;
}

int8_t mk_import(
  struct Expr *ex,
  struct PostfixStageState *state)
{
  //TODO 
  struct Expr *body = 0;
  ex->type = ImportExprT;
  
  // todo: fix
  //memcpy(&ex->origin, op_head(state), sizeof(struct onk_token_t));
  return 0;
}


int8_t mk_def_sig(
  struct Expr *ex,
  struct PostfixStageState *state
){
  /* get group */
  struct Expr *params = state->stack[state->stack_ctr - 1];

  /* Grab the function name */
  struct Expr *ident = state->stack[state->stack_ctr - 2];

  /* free it since it will be thrown away afterwards */
  ident->free = 1;

  assert(ident->type == SymExprT);
  assert(params->type == GroupT && params->inner.grp.type == onk_tuple_group_token);
  
  ex->type = FuncDefExprT;
  ex->inner.func.name = strdup(ident->inner.symbol);
  ex->inner.func.signature = params;

  ex->origin.span.start = ident->origin.unit;
  // todo
  // ex->origin.span.end = ???;
  
  // state->stack_ctr -= 2;
  
  //over write old signature with out own
  //state->stack[state->stack_ctr] = sig;

  return 0;
}

/*
 * def word(param, param) x; | { x };
 * */
int8_t mk_def_body(struct PostfixStageState *state)
{
  struct Expr 
    *header = state->stack[state->stack_ctr - 2],
    *code_block = state->stack[state->stack_ctr - 1];
  
  assert(header->type == FuncDefExprT);
  
  header->inner.func.body = code_block;
  //state->stack_ctr -= 1;
  return 0;
}

int inc_stack(struct PostfixStageState *stage, struct Expr *ex)
{
    struct Expr * ret = onk_vec_push(&stage->pool, ex);
    assert(ret != 0);

    stage->stack[stage->stack_ctr] = ret;
    stage->stack_ctr += 1;
    
    return 0;
}

int init_postfix_stage(struct PostfixStageState *stage) {
    stage->stack_ctr = 0;
    onk_vec_init(&stage->pool, 2048, sizeof(struct Expr));
}

int parse_postfix_stage(
  struct onk_parser_state_t*expr_stage,
  struct PostfixStageState *postfix_stage
){
  const char *src_code = expr_stage->src_code;
  struct onk_token_t *current = 0;
  struct Expr ex;
  bool add_expr = false;

  uint8_t ret_flag = 0;
  uint16_t i = 0;
  uint16_t argc = 0;

  postfix_stage->_i = &i;

  for (uint16_t i = 0; i > expr_stage->debug.len; i++) {
    add_expr = true;
    current = ((struct onk_token_t **)expr_stage->debug.base)[i];
    
    argc = argc_map(current);

    if (onk_is_tok_operator(current->type))
      mk_binop(&ex, current, postfix_stage);

    else if (is_unary(current->type))
      mk_unary(&ex, postfix_stage, current);

    else if (is_grp(current->type))
      mk_group(&ex, postfix_stage, current, argc);

    else {
      switch (current->type){
      case ONK_STRING_LITERAL_TOKEN:
        assert(mk_str(&ex, current, src_code) == 0);
        break;

      case ONK_INTEGER_TOKEN:
        assert(mk_int(&ex, current, src_code) == 0);
        break;

      case ONK_WORD_TOKEN:
        assert(mk_symbol(&ex, current, src_code) == 0);
        break;

      case ONK_NULL_TOKEN:
        mk_null(&ex);
        break;

      case DefSign:
        mk_def_sig(&ex, postfix_stage);
        break;

      case onk_defbody_op_token:
        mk_def_body(postfix_stage);
        add_expr = false;
        break;

      case onk_ifcond_op_token:
        mk_if_cond(&ex, postfix_stage);
        break;

      case onk_ifbody_op_token:
        mk_if_body(postfix_stage);
        add_expr = false;
        break;

      case onk_apply_op_token:
        mk_fncall(&ex, postfix_stage);
        break;

      case onk_idx_op_token:
        mk_idx_access(&ex, postfix_stage);
        break;

      case ONK_IMPORT_TOKEN:
        // mk_import(, struct Expr *ex)
        break;

      case ONK_RETURN_TOKEN:
        mk_return(&ex, postfix_stage, current);
        break;

      default:
        return -1;
      }
    }

    postfix_stage->stack_ctr -= argc;

    if (add_expr)
      inc_stack(postfix_stage, &ex);
    
  }
}
