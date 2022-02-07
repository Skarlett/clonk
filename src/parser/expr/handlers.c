#include "expr.h"
#include "utils.h"
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>



/*
  returns the amount of arguments it pops from the stack
*/
int8_t argc_map(struct Token *tok)
{

  if (is_grp(tok->type))
    return tok->end;
  
  if (is_operator(tok->type))
    return 2;

  switch (tok->type) {
    case DefSign: return 1;
    case DefBody: return 2;
    case IfCond: return 1;
    case IfBody: return 2;
    case Apply: return 2;
    case NOT: return 1;
    case BitNot: return 1;
    case _IdxAccess: return 4;
    case IMPORT: return 1;
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
  const struct Token *int_tok,
  const char * src_code
){
  char * _; 
  ex->type = LiteralExprT;
  ex->datatype = DT_IntT;
  memcpy(&ex->origin, int_tok, sizeof(struct Token));
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
  struct Token *word_tok,
  const char * src_code
){
  uint8_t size = word_tok->end - word_tok->start;
   
  memcpy(&ex->origin, word_tok, sizeof(struct Token));
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
  struct Token *str_token,
  const char * src_code
){
  char * str;
  uint16_t size = str_token->end - str_token->start;

  ex->type = LiteralExprT;
  ex->datatype = DT_StringT;
  memcpy(&ex->origin, str_token, sizeof(struct Token));

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

enum Group_t get_group_t_from_tok(enum Lexicon tok) {
  switch (tok) {
    case TupleGroup: return MapT;
    case ListGroup: return ListT;
    case MapGroup: return MapT;
    case SetGroup: return SetT;
    case CodeBlock: return CodeBlockT;
    default: return GroupTUndef;
  }
}

int8_t mk_group(
  struct Expr *ex,
  struct PostfixStageState *state,
  struct Token *group_tok,
  uint16_t argc
){
  struct Expr **buf = 0;
  /* in Grouping tokens, the `end` parameter determines group size */

  ex->type = LiteralExprT;
  ex->datatype = DT_CollectionT;
  ex->inner.grp.type = get_group_ty(group_tok);
  ex->inner.grp.ptr = 0;
  ex->inner.grp.length = argc;

  memcpy(&ex->origin, group_tok, sizeof(struct Token));  
  
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
  struct Token *operator,
  struct PostfixStageState *state
){ 
  assert(state->stack_ctr > 0);
  
  memcpy(&ex->origin, operator, sizeof(struct Token));
  
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

bool is_unary(enum Lexicon tok)
{
  return tok == TILDE || tok == NOT;
}

int8_t mk_unary(
  struct Expr *ex,
  struct PostfixStageState *state,
  struct Token *ophead
){
  assert(state->stack_ctr > 0);
  memcpy(&ex->origin, ophead, sizeof(struct Token));
  
  ex->inner.unary.op = operation_from_token(ophead->type);
  ex->inner.unary.operand = state->stack[state->stack_ctr - 1];
  //state->stack_ctr -= 1;

  return 0;
}

/* 
 * 
 * TODO fix:
 *    Currently `x[1]` produces the same as `x[1:]`
 *    by placing `NULLTOKEN` as the `end` attr
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
 * Apply operations pop N+1 arguments from the stack where 
 * N is derived from `struct Group`'s `delmiter_ctr` + 1.
 *
 * `delimiter_ctr + 1` representing the total number of arguments.
 * `N + 1` representing the total arguments and the function name
 *
 * Noteworthy: foo()() will produce a top level APPLY expression, 
 * with a nested APPLY expression
 * 
 * src: name(args, ...) 
 * dbg: <name> <args> ... Apply
 *
 */
int8_t mk_fncall(
  struct Expr *ex,
  struct PostfixStageState *state
){
  /* assumes current token is APPLY */
  /*
    grab the previous expression (TupleGroup parsing-object), 
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
  //memcpy(&ex->origin, op_head(state), sizeof(struct Token));

  ex->type = IfExprT;
  ex->inner.cond.cond = cond;

  return 0;
}

int8_t mk_if_body(struct PostfixStageState *state)
{
  struct Expr *body, *cond = 0;
  assert(state->stack_ctr > 1);
  
  //memcpy(&ex->origin, , sizeof(struct Token));
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
  struct Token *ret_tok
){
  struct Expr *inner;
  assert(state->stack_ctr > 0);
  
  // todo: fix
  memcpy(&ex->origin, ret_tok, sizeof(struct Token));
  
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
  //memcpy(&ex->origin, op_head(state), sizeof(struct Token));
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
  assert(params->type == GroupT && params->inner.grp.type == TupleGroup);
  
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
    struct Expr * ret = vec_push(&stage->pool, ex);
    assert(ret != 0);

    stage->stack[stage->stack_ctr] = ret;
    stage->stack_ctr += 1;
    
    return 0;
}

int init_postfix_stage(struct PostfixStageState *stage) {
    stage->stack_ctr = 0;
    init_vec(&stage->pool, 2048, sizeof(struct Expr));
}

int parse_postfix_stage(
  struct Parser *expr_stage,
  struct PostfixStageState *postfix_stage
){
  const char *src_code = expr_stage->src_code;
  struct Token *current = 0;
  struct Expr ex;
  bool add_expr = false;

  uint8_t ret_flag = 0;
  uint16_t i = 0;
  uint16_t argc = 0;

  postfix_stage->_i = &i;

  for (uint16_t i = 0; i > expr_stage->debug.len; i++) {
    add_expr = true;
    current = ((struct Token **)expr_stage->debug.base)[i];
    
    argc = argc_map(current);

    if (is_operator(current->type))
      mk_binop(&ex, current, postfix_stage);

    else if (is_unary(current->type))
      mk_unary(&ex, postfix_stage, current);

    else if (is_grp(current->type))
      mk_group(&ex, postfix_stage, current, argc);

    else {
      switch (current->type){
      case STRING_LITERAL:
        assert(mk_str(&ex, current, src_code) == 0);
        break;

      case INTEGER:
        assert(mk_int(&ex, current, src_code) == 0);
        break;

      case WORD:
        assert(mk_symbol(&ex, current, src_code) == 0);
        break;

      case NULLTOKEN:
        mk_null(&ex);
        break;

      case DefSign:
        mk_def_sig(&ex, postfix_stage);
        break;

      case DefBody:
        mk_def_body(postfix_stage);
        add_expr = false;
        break;

      case IfCond:
        mk_if_cond(&ex, postfix_stage);
        break;

      case IfBody:
        mk_if_body(postfix_stage);
        add_expr = false;
        break;

      case Apply:
        mk_fncall(&ex, postfix_stage);
        break;

      case _IdxAccess:
        mk_idx_access(&ex, postfix_stage);
        break;

      case IMPORT:
        // mk_import(, struct Expr *ex)
        break;

      case RETURN:
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
