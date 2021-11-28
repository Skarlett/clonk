#include <stdbool.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../../prelude.h"
#include "../lexer/lexer.h"
#include "../lexer/helpers.h"
#include "../lexer/debug.h"
#include "../../utils/vec.h"
#include "expr.h"
#include "expect.h"
#include "handlers.h"

int8_t mk_error(struct ExprParserState *state, enum ErrorT type, const char * msg) {
  struct CompileTimeError *err;
  
  err->type = type;
  err->msg = msg;
  if (vec_push(&state->errors, &state->src[*state->_i]) == 0)
    return -1;
  
  set_flag(&state->panic_flags, STATE_PANIC | STATE_INCOMPLETE);
  return 0;
}

int8_t throw_internal_error(struct ExprParserState *state, const char * meta, const char * msg)
{
  char * internal_msg;
  //TODO

#ifdef DEBUG
  internal_msg = malloc(strlen(meta) + strlen(msg));
  strcat(internal_msg, meta);
  strcat(internal_msg, msg);
#else
  internal_msg = msg;
#endif
  set_flag(&state->panic_flags, STATE_PANIC | STATE_INCOMPLETE | INTERNAL_ERROR);
  if (mk_error(state, Fatal, internal_msg) == -1)
    return -1;
  return 0;
}

#define throw_internal_error(X, MSG) throw_internal_error(X, FILE_LINE, MSG)

int8_t add_dbg_sym(
  struct ExprParserState *state,
  enum Lexicon type,
  uint16_t argc
){
  struct Token marker, *ret;
  if (type == TOKEN_UNDEFINED)
  {
    throw_internal_error(state, "got null token.");
    return -1;
  }

  marker.type = type;
  marker.start = 0;
  marker.end = argc;
  
  // push debug token in the pool
  ret = vec_push(&state->pool, &marker);
  
  // push pool ref into debug output
  if (ret == 0 || vec_push(&state->debug, &ret) == 0)  
  {
    throw_internal_error(state, "allocation failure in vec.");
    return -1;
  }
  return 0;
}

int8_t inc_stack(
  struct ExprParserState *state,
  struct Expr *ex,
  struct Token *dbg_out)
{
  struct Expr * heap_ex;
  
  if (state->expr_ctr > state->expr_sz)
  {
    throw_internal_error(state, "Expr/debug ctr overflowed.");
    return -1;
  }
  
  heap_ex = vec_push(&state->expr_pool, &ex);

  if (heap_ex == 0 || (dbg_out && vec_push(&state->debug, &dbg_out) == 0))
  {
    throw_internal_error(state, "vec pool returned null ptr.");
    return -1;
  }
  
  state->expr_stack[state->expr_ctr] = heap_ex;
  state->expr_ctr += 1;

  return 0;
}

struct Token * prev_token(struct ExprParserState *state) {
  if (UINT16_MAX - 1 > *state->_i && *state->_i != 0)
    return &state->src[*state->_i - 1];
  return 0;
}

struct Group * group_head(struct ExprParserState *state)
{
  if (STACK_SZ - 1 > state->set_ctr && state->set_ctr > 0)
    return &state->set_stack[state->set_ctr - 1];
  return 0;
 // return state->state[state->set_ctr - 1];
}

bool is_op_keyword(enum Lexicon token) 
{
  return token == IF
     || token == ELSE
     || token == RETURN
     || token == FUNC_DEF
     /* internal */
     || token == IfCond
     || token == IfBody
     || token == DefSign
     || token == DefBody;
}

/*
    precendense table:
      ") ] }"   : 127 non-assoc
      "."     : 126 L
      "^"       : 6 R (1 ^ 2 ^ 3) -> (1 ^ (2 ^ 3))
      "/ * %"   : 5 L  (4 / 2 * 2) -> ((4 / 2) * 2)
      "+ -"     : 4 L
      "!"       : 2 R
      ">> << | &"
      "!= == >= > <= < && ||": 2 L
      "+= -= =" : 1 R
      "( [ { IF ELSE RETURN DEF" : 0 non-assoc
*/
#define END_PRECEDENCE 127
int8_t op_precedence(enum Lexicon token) {
    
    if (is_close_brace(token))
        return END_PRECEDENCE;
    
    if (token == DOT)
        return 126;
    
    else if (token == POW)
        return 6;

    else if (token == MUL 
        || token == DIV
        || token == MOD)
        return 5;

    else if (token == ADD
        || token == SUB)
        return 4;
    
    else if (token == ISEQL
        || token == ISNEQL
        || token == GTEQ
        || token == LTEQ
        || token == GT
        || token == LT
        || token == AND
        || token == OR
        || token == NOT)
        return 2;
    
    else if (token == EQUAL
      || token == PLUSEQ
      || token == MINUSEQ)
      return 1;
    
    else if (is_open_brace(token) || is_op_keyword(token))
        return 0;
    
    return -1;
}

/*
 * Note: 
 * 
 * flush out keyword chains
 */
int8_t flush_keyword_chain(struct ExprParserState *state) 
{
  struct Token *head = state->operator_stack[state->operators_ctr - 1];
	  
  if(head->type == IfCond || !is_op_keyword(head->type))
    return -1;


  while(is_op_keyword(head->type)) 
  {
    
  }

  return 0;
}
/****
 *  Flushes operators out of stack 
 *  into the output until stack is empty
 *  or runs into an operator with 0 precedence 
 *  value.
*/
int8_t flush_ops_until_delim(struct ExprParserState *state)
{
  struct Token *head = 0;
  struct Expr ex;

  if (state->operators_ctr == 0)
    return 0;
  
  head = state->operator_stack[state->operators_ctr - 1];

  /* pop operators off of the operator-stack into the output */
  while (state->operators_ctr > 0) {
    memset(&ex, 0, sizeof(struct Expr));
    
    /* ends if tokens inverted brace is found*/
    if (op_precedence(head->type) == 0)
      break;
    
    /* otherwise pop into output */
    else {
      if (mk_operator(state, &ex, head) == -1 || inc_stack(state, &ex, head) == -1)
        return -1;
      
      state->operators_ctr -= 1;
    }

    if (state->operators_ctr <= 0)
      break;

    /* Grab the head of the stack */
    head = state->operator_stack[state->operators_ctr - 1];
  }
  
  return 0;
}

int8_t flush_all_ops(struct ExprParserState *state)
{
  /* dump the remaining operators onto the output */
  struct Token *head;
  struct Expr ex;

  while (state->operators_ctr > 0)
  {
    memset(&ex, 0, sizeof(struct Expr));

    head = state->operator_stack[state->operators_ctr - 1];
    /*
        any remaining params/brackets/braces are unclosed
        indiciate invalid expressions
    */
    if (is_open_brace(head->type))
      return -1;

    if (mk_operator(state, &ex, head) == -1
        || inc_stack(state, &ex, head) == -1)
        return -1;
  
    state->operators_ctr -= 1;
  }

  return 0;
}

enum Associativity
{
    NONASSOC,
    RASSOC,
    LASSOC
};

enum Associativity get_assoc(enum Lexicon token)
{
    switch(token) {
        case POW:
            return RASSOC;
        case NOT:
            return RASSOC;
        default:
            return LASSOC;
    }
}

/* 
 * When an operator is placed in the parser,
 * it's order precedence is determined
 * using shunting yard.
 * 
 * Shunting yard uses an operator stack to 
 * determine evaluation order. If the precedence
 * of the current operator (op1) is less than 
 * whats at the top/head of the stack (op2) then
 * we'll drop operations out of the stack 
 * onto the output until the precedense of the op1 is
 * equal (unless right-assciotated).
 * or greater than op2.
 * 
 * When operators are placed onto the output,
 * they're created into expression. Each 
 * binary operator will pop 2 arguments 
 * from the top of the expression stack, 
 * and then the operator expression will
 * be pushed onto the expression stack.
 *
 * postfix: ... <expr> <expr> <OPERATOR> ...
 */
int8_t handle_operator(struct ExprParserState *state) {
  struct Expr ex;
  struct Token *head=0, *current=0;
  int8_t precedense = 0, head_precedense = 0;

  current = &state->src[*state->_i];
  
  /* TODO add to expectation function instead */
  /* if/else are placed in the operator-stack */
  /* if (current->type == ELSE && !check_flag(state->ctx, STATE_CTX_ACCEPT_ELSE)) */
  /*   return -1; */
  
  /* else if (current->type == IF) */
  /*   set_flag(&state->ctx, STATE_CTX_IF_HEAD | STATE_CTX_IF_BODY); */
    
  /* else if (current->type == ELSE) */
  /*   set_flag(&state->ctx, STATE_CTX_ELSE_BODY); */
      
  /* else if (current->type == RETURN) */
  /*   set_flag(&state->ctx, STATE_CTX_RETURN_BODY); */
  
  /*
    If no operators are in operators-stack,
    place it directly.

    If the head of the operator stack is an open brace
    we don't need to do anymore checks
    before placing the operator  
  */  
  if (state->operators_ctr == 0 || is_open_brace(head->type))
  {
    state->operator_stack[state->operators_ctr] = current;
    state->operators_ctr += 1;
    return 0;
  }

  /* Grab the head of the operators-stack */
  head = state->operator_stack[state->operators_ctr-1];
  precedense = op_precedence(current->type);
  head_precedense = op_precedence(head->type);

  if (precedense == -1 || head_precedense == -1){
    throw_internal_error(state, "Unrecognized operator");
    return -1;
  }

  /*
    while `head` has higher precedence
    than our current token pop operators from
    the operator-stack into the output
  */
  while (head_precedense > 0 && head_precedense >= precedense && state->operators_ctr > 0) {
    memset(&ex, 0, sizeof(struct Expr));

    if (head_precedense == 0)
      break;
    
    /* pop operators off the stack into the output */
    if (head_precedense > precedense)
    {
      if (mk_operator(state, &ex, head) == -1 
        || inc_stack(state, &ex, head) == -1)
        return -1;
    }
    /*
        If left associated, push equal
        precedence operators onto the output
    */
    else if (precedense == head_precedense && get_assoc(current->type) == LASSOC) 
    {
       if (mk_operator(state, &ex, head) == -1
          || inc_stack(state, &ex, head) == -1)
          return -1;
    }

    /* discard operator after placed in output */
    state->operators_ctr -= 1;

    if (state->operators_ctr <= 0)
      break;

    head = state->operator_stack[state->operators_ctr - 1];
    head_precedense = op_precedence(head->type);
  }

  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  return 0;
}

int8_t push_op(enum Lexicon op, uint16_t start, uint16_t end, struct ExprParserState *state)
{
  struct Token new, *heap = 0;
  new.type = op;
  new.end = end;
  new.start = start;

  heap = vec_push(&state->pool, &new);
  
  if (!heap)
    return -1;
  
  state->operator_stack[state->operators_ctr] = heap;  
  state->operators_ctr += 1;
  return 0;
}

int8_t peek_behind_operator(struct ExprParserState *state)
{
  struct Token *current=0, *prev = 0;  
  current = &state->src[*state->_i];
  prev = prev_token(state);

  if (prev)
  {
    /* function call pattern */
    if (current->type == PARAM_OPEN
      && (is_close_brace(prev->type) || prev->type == WORD)
      && push_op(Apply, 0, 0, state) == -1)
      return -1;
  
  /* index call pattern */
    else if (current->type == BRACKET_OPEN)
    {
      /* peek-behind to check for index access */
      if ((is_close_brace(prev->type)
         || prev->type == WORD
         || prev->type == STRING_LITERAL)
         && push_op(_IdxAccess, 0, 0, state) == -1)
         return -1;
    }
  }
  return 0;
}

/*
  every opening brace starts a new possible group,
  increment the group stack, and watch for the following 
  in-fix patterns.
   
  accepts the following patterns as function calls
  where the current token is `(`
       )(
       ](
    word(
        ^-- current token.

  accepts the following patterns as an array index
    where the current token is `[`
        )[
        ][
     word[
        "[
         ^-- current token.
*/
int8_t handle_open_brace(struct ExprParserState *state) {
  struct Group *ghead = 0;
  struct Token *current;

  current = &state->src[*state->_i];

  /* overflow check */
  if (state->set_ctr > state->set_sz ||
      state->operators_ctr > state->operator_stack_sz)
  {
    throw_internal_error(state, "Internal group/operator stack overflowed.");
    return -1;
  }
  
  ghead = &state->set_stack[state->set_ctr];
  /* increment group */
  state->set_ctr += 1;

  ghead->delimiter_cnt = 0;
  ghead->origin = current;
  
  /* look behind to determine if */
  if (peek_behind_operator(state) == -1)
    return -1;

  // Place opening brace on operator stack
  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;
  ghead->state = GSTATE_CTX_DATA_GRP; 

  return 0;
}

int8_t pad_idx_op()
{
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  struct Token *prev = prev_token(state);

  if (ghead->origin->type != BRACE_OPEN || !prev);
      return -1;
  
  mk_null(&ex);
    
  /*
    peek-behind if token was COLON 
    add a value for its missing argument
  */
  // a: -> a:a
  // :: => ::a
  
  if (prev->type == COLON && 
    (add_dbg_sym(state, NULLTOKEN, 0) == -1 
    || inc_stack(state, &ex, 0) == -1))
    return -1;
  
  // a:a -> a:a:a
  for (uint8_t i=0; 2 > ghead->delimiter_cnt; i++)
  {
    if(add_dbg_sym(state, NULLTOKEN, 0) == -1 
      || inc_stack(state, &ex, 0) == -1)
        return -1;
  }

  memset(&ex, 0, sizeof(struct Expr));
  if (add_dbg_sym(state, _IdxAccess, 0) == -1
      || mk_idx_access(state, &ex) == -1
      || inc_stack(state, &ex, 0) == -1)
        return -1;
  
  return 0;
}

int8_t handle_fncall(struct ExprParserState *state) {
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  struct Expr ex;

  if (ghead->origin->type != PARAM_OPEN 
      || add_dbg_sym(state, Apply, ghead->delimiter_cnt + 1) == -1
      || mk_fncall(state, &ex) == -1
      || inc_stack(state, &ex, 0) == -1)
      return -1;
  return 0;
}

enum GroupT get_group_ty(struct Group *ghead) {
  if (check_flag(ghead->state, GSTATE_CTX_MAP_GRP))
    return MapT;

  else if(check_flag(ghead->state, GSTATE_CTX_CODE_GRP))
    return CodeBlockT;

  if (ghead->origin->type == BRACKET_OPEN)
    return ListT;

  else if (ghead->origin->type == PARAM_OPEN)
    return TupleT;

  else if(ghead->origin->type == BRACE_OPEN && check_flag(ghead->state, GSTATE_CTX_DATA_GRP))
    return SetT;
  
  else
    return GroupTUndef;
}
/******************
 * Index operation *
 ******************
 * INDEX_ACCESS acts as a function in the postfix representation
 * that pops 4 arugments from the stack
 * `source`, `start`, `end`, `skip` in that order.
 * 
 * INDEX_ACCESS every argument except `source`
 * may be substituted with NULLTOKENS.
 *
 * NULLTOKENS can inserted automatically by parser or operator.
 *
 * NULLTOKENS used as substitution will assume 
 * their default values as the following.
 * 
 * `start` defaults to 0.
 * `end` defaults to length of the array.
 * `skip` defaults to 1.
 * 
 * Examples:
 *   token output:
 *     WORD   INTEGER INTEGER INTEGER INDEX_ACCESS 
 *     source start   end     skip    operator
 *   
 *   text:
 *     foo[1::2]
 *   
 *   postfix-IR: foo 1 NULL 2 INDEX_ACCESS
 *   
 *
 * src: name[args:args:args] 
 * dbg: <name> <args> ... IdxAccess
 */
int8_t handle_idx_op(struct ExprParserState *state) {
  struct Expr ex;
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  struct Token *prev = &state->src[*state->_i - 1];

  if (ghead->origin->type != BRACE_OPEN)
      return -1;
    
  mk_null(&ex);
    
  /*
    peek-behind if token was COLON 
    add a value for its missing argument
  */
  // a: -> a:a
  // :: => ::a
  if (prev->type == COLON && 
    (add_dbg_sym(state, NULLTOKEN, 0) == -1 
    || inc_stack(state, &ex, 0) == -1))
    return -1;
  
  // a:a -> a:a:a
  for (uint8_t i=0; 2 > ghead->delimiter_cnt; i++)
  {
    if(add_dbg_sym(state, NULLTOKEN, 0) == -1 
      || inc_stack(state, &ex, 0) == -1)
        return -1;
  }

  memset(&ex, 0, sizeof(struct Expr));
  if (add_dbg_sym(state, _IdxAccess, 0) == -1
      || mk_idx_access(state, &ex) == -1
      || inc_stack(state, &ex, 0) == -1)
        return -1;
  
  return 0;
}

int8_t handle_fncall(struct ExprParserState *state) {
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  struct Expr ex;

  if (ghead->origin->type != PARAM_OPEN 
      || add_dbg_sym(state, Apply, ghead->delimiter_cnt + 1) == -1
      || mk_fncall(state, &ex) == -1
      || inc_stack(state, &ex, 0) == -1)
      return -1;
  return 0;
}

enum Lexicon grp_dbg_sym(enum GroupT type){
  switch (type) {
    case ListT: return ListGroup;
    case SetT: return SetGroup;
    case MapT: return MapGroup;
    case CodeBlockT: return CodeBlock;
    case TupleT: return TupleGroup;
    default: return TOKEN_UNDEFINED;
  };
}

/* 
 * Groups take N amount of arguments from the stack where 
 * N is derived from `struct Group`'s delmiter_ctr + 1.
 *
 * Grouping represents sets of data like
 * lists, tuples, & codeblocks. 
 *
 * src: [body_expr, ...]
 * dbg: <body-expr> ... GroupType
 */
int8_t handle_grouping(struct ExprParserState *state) {
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  enum Lexicon marker_type;
  struct Expr ex;

  /* only add groups if they're not singular item paramethesis braced */
  if (ghead->origin->type != PARAM_OPEN 
      || ghead->delimiter_cnt > 0
      || check_flag(ghead->state, GSTATE_EMPTY))
      if (mk_group(state, &ex) == -1
        || add_dbg_sym(state, grp_dbg_sym(ex.inner.value.literal.grouping.type), ghead->delimiter_cnt + 1) == -1
        || inc_stack(state, &ex, 0) == -1)
        return -1;
  return 0;
}

int8_t handle_unit_expr(struct ExprParserState *state) {
  struct Token *next, *current = &state->src[*state->_i];
  struct Expr ex;
  

  /* only add groups if they're not singular item paramethesis braced */
  if (current->type == INTEGER && mk_int(state, &ex) == -1)
    return -1;

  else if (current->type == STRING_LITERAL && mk_str(state, &ex) == -1)
     return -1;
  
  else if (current->type == WORD
    && mk_symbol(state, &ex) == -1)
    return -1;
  }
  
  return inc_stack(state, &ex, current);
}


/* 
 * `if` pops 2 arguments off the stack.
 * The first is the conditional expression,
 * the second is the condition's body.
 *
 * README: Expects an `IF` token at the top of
 * the operator stack.
 * 
 * src: if(cond-expr) body-expr
 * dbg: <cond-expr> <body-expr> IF
 */
int8_t handle_else_body_grp(struct ExprParserState *state) {
  struct Expr *else_body_ex, *if_ex;
  struct Group *ghead;
  struct Token *ophead;

  if (1 > state->operators_ctr)
    return -1;
  
  ophead = state->operator_stack[state->operators_ctr - 1];

  if (ophead->type != ELSE 
    || 2 > state->expr_ctr
    || handle_grouping(state) == -1)
    return -1;
  
  else_body_ex = state->expr_stack[state->expr_ctr - 1];
  if_ex = state->expr_stack[state->expr_ctr - 2];
  if_ex->inner.cond.else_body = else_body_ex;

  /* drop remove else body from expr-stack */
  state->expr_ctr -= 1;

  /* drop else token out of operator-stack */
  state->operators_ctr -= 1;

  return 0;
}

int8_t handle_if(struct ExprParserState *state) {
  struct Token *ophead = state->operator_stack[state->operators_ctr - 1];
  struct Expr ex; 
  struct Token tmp, *cond, *body;

  tmp.end = 0;
  tmp.start = 0;
  tmp.type = IfCond;
  cond = vec_push(&state->pool, &tmp);
  
  tmp.type = IfBody;
  body = vec_push(&state->pool, &tmp);

  state->operator_stack[state->operators_ctr] = body;
  state->operator_stack[state->operators_ctr + 1] = cond;
  state->operators_ctr += 2;

  // set_flag(&state->ctx, STATE_CTX_ACCEPT_ELSE);
  return 0;
}

int8_t handle_close_brace(struct ExprParserState *state) {
  struct Token *prev = 0, *ophead=0, *current = &state->src[*state->_i];
  struct Group *ghead;
  struct Expr ex;
  int8_t ret = 0;

  enum Lexicon marker_type = TOKEN_UNDEFINED;
  
  if (state->set_ctr == 0) {
    mk_error(state, Fatal, "Unexpected closing brace.");
    return -1;
  }
   /* Operators stack is empty */
  else if (0 > state->operators_ctr
    /* unbalanced brace, extra closing brace.*/
    || state->set_ctr == 0
    || state->set_ctr > state->set_sz)
    return -1;

  if (*state->_i > 0)
    prev = &state->src[*state->_i - 1];
  else
    return -1;

  /* Grab the head of the group stack */
  ghead = &state->set_stack[state->set_ctr - 1];
  
  /* grab head of operator stack */
  ophead = state->operator_stack[state->operators_ctr - 1];

  /* setup is empty flag */
  if (prev->type == invert_brace_tok_ty(current->type)) {
    set_flag(&ghead->state, GSTATE_EMPTY);
    state->operators_ctr -= 1;
    
    /* on index ctx throw error */
    if (check_flag(ghead->state, GSTATE_CTX_IDX)) {
      mk_error(state, Error, "Slice must contain atleast one delimiter or value.");
      return -1;
    }

    return 0; 
  }
  
  /*
    flush out operators, until the 
    open-brace type is found 
    in the operator-stack.
  */
  else if (flush_ops_until_delim(state) == -1)
      return -1;
/*
 *
 * (((x if) (((y if) ((((z if) a body)) b else) body) c else) body) d else) 
 *  
 *
 * handle If condition/body
*/
  else if(ophead->type == IfCond)
  { 
    
    // operators_ctr -= 1; 
  }

  // TODO Remove context flags
	
  /* handle function call */
  /* else if (check_flag(ghead->state,  GSTATE_OP_APPLY)) */
  /*   ret = handle_fncall(state); */
  
  /* /1* handle if condition *1/ */
  /* else if (check_flag(ghead->state,  GSTATE_CTX_IF_COND)) */
  /*   nop; /1* yes, do nothing *1/ */
  
  /* /1* handle if-body *1/ */
  /* else if (check_flag(ghead->state,  GSTATE_CTX_IF_BODY)) */
  /*   ret = handle_if_body_grp(state); */

  /* /1* handle else *1/ */
  /* else if (check_flag(ghead->state,  GSTATE_CTX_ELSE_BODY)) */
  /*   ret = handle_else_body_grp(state); */

  /* /1* handle indexing *1/ */
  /* else if (check_flag(ghead->state, GSTATE_CTX_IDX)) */
  /*   ret = handle_idx_op(state); */
  
  /* /1* handle grouping *1/ */
  /* else if (check_flag(ghead->state, GSTATE_CTX_CODE_GRP */ 
  /*   | GSTATE_CTX_DATA_GRP | GSTATE_CTX_MAP_GRP)) */
  /*     ret = handle_grouping(state); */
  /* else */ 
  /* { */
  /*   throw_internal_error(state, "got null token."); */
  /*   return -1; */
  /* } */

  /* remove head of stack */
  state->set_ctr -= 1;
  return ret;
}

int8_t update_ctx(enum Lexicon delimiter, struct Group *ghead){
  if (check_flag(ghead->state, GSTATE_CTX_LOCK))
    return 0;
  
  if (delimiter == COMMA)
    set_flag(&ghead->state, GSTATE_CTX_DATA_GRP | GSTATE_CTX_LOCK);

  else if (delimiter == SEMICOLON)
    set_flag(&ghead->state, GSTATE_CTX_CODE_GRP | GSTATE_CTX_LOCK);

  else if (delimiter == COLON) {
    if(ghead->origin->type == BRACE_OPEN)
      set_flag(&ghead->state, GSTATE_CTX_MAP_GRP | GSTATE_CTX_LOCK);
    else return -1;
  }
  else return -1;
  return 0;
}

int8_t handle_delimiter(struct ExprParserState *state) {
  struct Expr ex;
  struct Token *head = 0, 
    *current = &state->src[*state->_i],
    *prev = 0,
    *next = 0;
  struct Group *ghead = 0;

  FLAG_T ret_flag;
  
  /* Setup group group head ptr */
  if (state->set_ctr > 0)
    ghead = &state->set_stack[state->set_ctr - 1];
  else
    ghead = &state->set_stack[0];
  
  if (check_flag(ghead->state, GSTATE_CTX_IF_COND))
    return -1;

  ghead->delimiter_cnt += 1;
  ghead->last_delim = current;
  
  if(*state->_i > 0)
    prev = &state->src[*state->_i - 1];
  
  if(state->src_sz > *state->_i + 1)
    next = &state->src[*state->_i + 1];

  if (!next || !prev) {
    mk_error(state, Fatal, "Expected token before & after delimiter.");
    return -1;
  }

  // TODO use operator stack instead of context 
  /* 
    IDX/APPLY operations will already have the context locked.
  */
  /* if (update_ctx(current->type, ghead) == -1) */
  /*   return -1; */
  
  /* /* peek behind and add NULL if no */*/
  /* if (check_flag(ghead->state, GSTATE_CTX_IDX))*/
  /* {*/
  /*   if(ghead->delimiter_cnt > 2)*/
  /*   {*/
  /*     throw_internal_error(state, "Not enough items to create idx operation.");*/
  /*     return -1;*/
  /*   }*/

  /*   /* add NOP to the output if no position argument present*/*/
  /*   if (prev == ghead->origin || prev->type == COLON)*/
  /*   {*/ 
  /*     mk_null(&ex);*/
  /*     if (add_dbg_sym(state, NULLTOKEN, 0) == -1*/
  /*        ||inc_stack(state, &ex, 0))*/
  /*       return -1;*/
  /*   }*/
  /* }*/

  // assumes brace 
  if (flush_ops_until_delim(state) == -1)
    return -1;

  /* syntax sugar for if/else */
  /* if((ret_flag = check_flag(state->ctx, STATE_CTX_IF_BODY | STATE_CTX_ELSE_BODY))) { */
  /*   if(ret_flag & STATE_CTX_IF_BODY) { */
  /*     unset_flag(&state->ctx, STATE_CTX_IF_BODY); */
  /*     handle_if_body_grp(state); */
  /*   } */
  /*   else */
  /*   { */
  /*     unset_flag(&state->ctx, STATE_CTX_ELSE_BODY); */
  /*     handle_else_body_grp(state); */
  /*   } */
  /* } */
  return 0;
}

// int8_t unwind_operator(struct ExprParserState *state, struct Expr *ex) {
//   struct Expr *op = state->expr_stack[state->expr_ctr - 1];

//   state->expr_stack[state->expr_ctr - 1] = ex->inner.bin.lhs;
//   state->expr_stack[state->expr_ctr] = ex->inner.bin.rhs;
//   state->expr_ctr += 1;
//   //struct Token *current = &state->src[*state->i - 1];
// }

/*
int8_t handle_unwind(struct ExprParserState *state) {

  struct Token *prev = &state->src[*state->i - 1],
              *start, *end;
  struct Group *ghead = 0;
   
  if (state->set_ctr > 0)
    ghead = &state->set_stack[state->set_ctr - 1];
  else
   return -1;

  //struct Token *spool_head = state->debug[*state->debug_ctr - 1];
  
  if (ghead->last_delim != 0)
    start = ghead->last_delim;

  else if (ghead->origin != 0)
    start = ghead->origin;

  else {}

  for (uint16_t i = *state->i; state->src_sz > i; i++)
  {

    if (state->src[i].type == COLON || state->src[i].type == COMMA)
    {
      
    }
  }
  return 0;
}
*/
int8_t initalize_parser_state(
    char * line,
    struct Token tokens[],
    uint16_t ntokens,
    uint16_t *i,
    struct ExprParserState *state
){
  if (init_vec(&state->expr_pool, 2048, sizeof(struct Expr)) == -1
    ||init_vec(&state->pool, 256, sizeof(struct Token)) == -1
    ||init_vec(&state->debug, 2048, sizeof(void *)) == -1
    ||init_vec(&state->errors, 64, sizeof(struct CompileTimeError)) == -1)
    return -1;
  
  state->line = line;
  state->_i = i;

  state->src = tokens;
  state->src_sz = ntokens;

  state->expr_ctr = 0;
  state->expr_sz = STACK_SZ;

  state->set_ctr = 0;
  state->set_sz = STACK_SZ;

  state->operators_ctr = 0;
  state->operator_stack_sz = STACK_SZ;
  state->panic_flags = 0;

  state->expecting_ref = state->expecting;
  init_expect_buffer(state);
  
  set_flag(&state->panic_flags, STATE_READY);
  return 0;
}

bool is_unit_expr(enum Lexicon tok)
{
  return \
    tok == STRING_LITERAL 
    || tok == INTEGER
    || tok == WORD;
}

bool is_terminal(enum Lexicon tok)
{
  return \
    tok == SEMICOLON
    || tok == COLON
    || tok == COMMA
    || is_close_brace(tok);
}

bool push_as_op(enum Lexicon tok)
{
  return \ 
    is_operator(tok)
    //|| tok == IF
    //|| tok == FUNC_DEF
    || tok == ELSE
    || tok == RETURN
}

int8_t parse_expr(
    char * line,
    struct Token tokens[],
    uint16_t expr_size,
    struct ExprParserState *state,
    struct Expr *ret
){
  uint16_t i = 0;

  if (expr_size == 0 || initalize_parser_state(line, tokens, expr_size, &i, state) == -1)
    return -1;

  for (i = 0; expr_size > i; i++) {
    if (state->expr_ctr > state->expr_sz
        || state->operators_ctr > state->operator_stack_sz
        // || state->ctx == FLAG_ERROR 
        || state->panic_flags == FLAG_ERROR)
        return -1;

    if (check_flag(state->panic_flags, STATE_PANIC) || is_token_unexpected(state))
    {
      // TODO
      // handle_unwind(&state);
      return -1;
    }
    
    /* string, word, integers */
    else if(is_unit_expr(state->src[i].type))
      handle_unit_expr(state);

    else if (push_as_op(state->src[i].type))
      handle_operator(state);
    
    else if (is_close_brace(state->src[i].type))
      handle_close_brace(state);

    else if (is_open_brace(state->src[i].type))
      handle_open_brace(state);

    else if (state->src[i].type == COLON 
      || state->src[i].type == COMMA
      || state->src[i].type == SEMICOLON)
      handle_delimiter(state);

    else if (state->src[i].type == FUNC_DEF)
     // TODO
     nop;
    else if (state->src[i].type == IF)
      handle_if(state);
    
    else if(state->src[i].type == EOFT)
	break;

    else {
#ifdef DEBUG
      printf("debug: token fell through precedence\n",
             ptoken(tokens[i].type));
#endif
    }
    /* setup next-token expectation */
    // state->expecting = expecting_next(state);
    // unset_flag(&state->ctx, STATE_CTX_ACCEPT_ELSE);
  }
  
  if(check_flag(state->panic_flags, STATE_INCOMPLETE))
    return -1;
  
  /* -1 from state._i so that it points to the last item in the input */
  *state->_i = state->src_sz-1;
  
  /* dump the remaining operators onto the output */
  if (flush_all_ops(state) == -1
    || state->expr_ctr != 1)
    return -1;
  
  ret = ((struct Expr **)(state->expr_pool.base))[0];

  return 0;
}

int8_t free_state(struct ExprParserState *state) {
  if (vec_free(&state->expr_pool) == -1
  || vec_free(&state->debug) == -1
  || vec_free(&state->pool) == -1
  || vec_free(&state->errors) == -1)
    return -1;
  
  return 0;
}

int8_t reset_state(struct ExprParserState *state) {
  
  memset(state->expr_stack, 0, sizeof(void *[STACK_SZ]));
  state->expr_ctr = 0;
  
  memset(state->operator_stack, 0, sizeof(void *[STACK_SZ]));
  state->operators_ctr = 0;
  
  memset(state->set_stack, 0, sizeof(void *[STACK_SZ]));
  state->set_ctr = 0;
  


  state->src_sz = 0;
  state->src = 0;
  state->_i = 0;
  state->line = 0;

  state->expecting_ref = 0;
  state->panic_flags = 0;
  
  free_state(state);
  return 0;
}
