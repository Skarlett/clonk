#include <stdbool.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "../../prelude.h"
#include "../lexer/lexer.h"
#include "../lexer/debug.h"
#include "../../utils/vec.h"
#include "expr.h"
#include "utils.h"

//#include "handlers.h"

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
  
  precedense = op_precedence(current->type);
  current = &state->src[*state->_i];
  /*
    If no operators are in operators-stack,
    place it directly.

    If the head of the operator stack is an open brace
    we don't need to do anymore checks
    before placing the operator  
  */  
  if (state->operators_ctr == 0 || precedense == 0)
  {
    state->operator_stack[state->operators_ctr] = current;
    state->operators_ctr += 1;
    return 0;
  }

  /* Grab the head of the operators-stack */
  head = state->operator_stack[state->operators_ctr-1];
  head_precedense = op_precedence(head->type);

  assert(precedense != -1 || head_precedense != -1);

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

/* for open param - checks if prev token was word/brace to determine if function call*/
int8_t handle_brace_op(struct ExprParserState *state)
{
  struct Token *current=0, *prev = 0;  
  
  current = &state->src[*state->_i];
  prev = prev_token(state);

  if (prev)
  {
    /* function call pattern */
    if (current->type == PARAM_OPEN
      && (is_close_brace(prev->type) || prev->type == WORD)
      && op_push(Apply, 0, 0, state) == 0)
      return -1;
  
  /* index call pattern */
    else if (current->type == BRACKET_OPEN)
    {
      /* is indexable */
      if ((is_close_brace(prev->type)
         || prev->type == WORD
         || prev->type == STRING_LITERAL)
         && op_push(_IdxAccess, 0, 0, state) == 0)
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
int8_t handle_open_brace(struct ExprParserState *state)
{
  struct Token *current;

  current = &state->src[*state->_i];

  /* overflow check */
  assert(state->operators_ctr < state->operator_stack_sz);
  
  /* look behind to determine special operators (apply/idx_access) */
  if (handle_brace_op(state) == -1 
      /* then create a new scope/group */
      || new_grp(state, current) == 0)
      return -1;

  // Place opening brace on operator stack
  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  return 0;
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
int8_t handle_idx_op(struct ExprParserState *state)
{
  struct Expr ex;
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  struct Token *prev = &state->src[*state->_i - 1];
    
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
int8_t handle_grouping(struct ExprParserState *state)
{
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  struct Expr ex;

  /* only add groups if they're not singular item paramethesis braced */
  if (ghead->origin->type != PARAM_OPEN 
      || ghead->delimiter_cnt > 0
      || ghead->state & GSTATE_EMPTY)
      if (mk_group(state, &ex) == -1
        || add_dbg_sym(state, grp_dbg_sym(ex.inner.value.literal.grouping.type), ghead->delimiter_cnt + 1) == -1
        || inc_stack(state, &ex, 0) == -1)
        return -1;
  return 0;
}

int8_t handle_unit_expr(struct ExprParserState *state)
{
  struct Token *current = &state->src[*state->_i];
  struct Expr ex;
  
  /* only add groups if they're not singular item paramethesis braced */
  if (current->type == INTEGER && mk_int(state, &ex) == -1)
    return -1;

  else if (current->type == STRING_LITERAL && mk_str(state, &ex) == -1)
     return -1;
  
  else if (current->type == WORD
    && mk_symbol(state, &ex) == -1)
    return -1;
   
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
int8_t handle_if(struct ExprParserState *state)
{
  struct Token *current = &state->src[*state->_i];
  enum Lexicon ops[] = {IfBody, IfCond, 0};
  
  return push_many_ops(ops, current, state);
}

int8_t handle_else(struct ExprParserState *state)
{
  struct Token *current = &state->src[*state->_i];

  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;
  return 0;
}

int8_t handle_def(struct ExprParserState *state)
{
  struct Token *current = &state->src[*state->_i];
  enum Lexicon ops[] = {DefBody, DefSign, 0};
  
  return push_many_ops(ops, current, state);
}

/*
 * Short group are groups 
 * that are unbraced and 
 * with one expression in it.
 *
 * Normal block-group
 * if(x) { x+1; } 
 *
 * Short block-group
 * if(x) x+1;
 *       ^
 *       ^
 * Short groups are popped off of the stack at the first 
 * delimiter found in their scope.
*/
struct Group * mk_short_block(struct ExprParserState *state)
{
  struct Token *ophead;
  struct Group *ghead;

  ophead = op_push(BRACE_OPEN, 0, 0, state);
  if (!ophead)
     return 0;
 
  ghead = new_grp(state, ophead);
  if(!ghead)
     return 0;

  ghead->state |= GSTATE_CTX_SHORT_BLOCK;
  
  return ghead;
}

int8_t pop_block_operator(struct ExprParserState *state)
{
  struct Token *ophead, *next;
  struct Expr ex;
  bool mk_short_blk = false;

  if(state->operators_ctr > state->operator_stack_sz
    || state->operators_ctr == 0)
    return -1;
  
  ophead = state->operator_stack[state->operators_ctr - 1];
  state->operators_ctr -= 1;

  // check for next token
  next = next_token(state);
  
  //TODO: ensure its not a data-collection
  if (!next)
    return 0;
    
  // specify short block
  //mk_short_blk = next 
  //  && next->type != BRACE_OPEN 
  // && is_short_blockable(next->type);

  switch (ophead->type)
  {
    case IfCond:
      if(mk_if_cond(state, &ex) == -1
        || inc_stack(state, &ex, ophead) == -1)
        return -1;
      break;
    
    case IfBody:
      if(mk_if_body(state) == -1
        || add_dbg_sym(state, ophead->type, 0) == -1)
        return -1;
      break;

    case ELSE:
      if(mk_else_body(state) == -1
        || add_dbg_sym(state, ophead->type, 0) == -1)
        return -1;
      break;
    
    case RETURN:
      if(mk_return(state, &ex) == -1 
        || inc_stack(state, &ex, ophead) == -1)
        return -1;
      break;

    case DefSign:
      if(mk_def_sig(state, &ex) == -1
        || inc_stack(state, &ex, ophead) == -1)
        return -1;
      break;

    case DefBody:     
      if(mk_def_body(state) == -1
        || add_dbg_sym(state, ophead->type, 0))
        return -1;
      break;

    case IMPORT:
      if(mk_import(state, &ex) == -1
	      || inc_stack(state, &ex, ophead) == -1)
	      return -1;
      break;
    default: return -1;
  }
 
  //if (mk_short_blk && mk_short_block(state) == 0)
  //  return -1;
  
  return 0;
}

int8_t handle_close_brace(struct ExprParserState *state) {
  struct Token *next = 0,
	       *prev = 0,
	       *ophead=0,
	       *current = &state->src[*state->_i];
  
  struct Group *ghead;
  struct Expr ex;
  int8_t ret = 0;

  enum Lexicon marker_type = TOKEN_UNDEFINED;
  
  prev = prev_token(state);

  if (state->set_ctr == 0) {
    mk_error(state, Fatal, "Unexpected closing brace.");
    return -1;
  }
   /* Operators stack is empty */
  else if (0 == state->operators_ctr
    || !prev
    /* unbalanced brace, extra closing brace.*/
    || state->set_ctr == 0
    || state->set_ctr > state->set_sz)
    return -1;

  /* Grab the head of the group stack */
  ghead = &state->set_stack[state->set_ctr - 1];
  state->set_ctr -= 1;

  /*
    flush out operators, until the 
    open-brace type is found 
    in the operator-stack.
  */
  if (flush_ops_until_delim(state) == -1)
      return -1;

  /* is empty ? */
  if (prev->type == invert_brace_tok_ty(current->type)) {
    ghead->state |= GSTATE_EMPTY;
    
    /* on index ctx throw error */
    if (prev->type == _IdxAccess) {
      mk_error(state, Error, "Slice must contain atleast one delimiter or value.");
      return -1;
    }

    state->operators_ctr -= 1;
    return 0; 
  }
  
  /* handle grouping */
  if (!(ghead->state & GSTATE_CTX_IDX))
     ret = handle_grouping(state); 
  
  /* drop open brace */
  state->operators_ctr -= 1;
  
  if (state->operators_ctr == 0)
    return 0;

  /* grab head of operator stack */
  ophead = state->operator_stack[state->operators_ctr - 1];
   
  /* handle If condition/body */
  if (ophead->type == Apply)
  {
    if (mk_fncall(state, &ex) == -1
	     || inc_stack(state, &ex, ophead) == -1)
	     return -1;
    state->operators_ctr -= 1;
  }

  else if(ophead->type == _IdxAccess)
  {
    if(mk_idx_access(state, &ex) == -1 
      || inc_stack(state, &ex, ophead) == -1)
      return -1;
    state->operators_ctr -= 1;
  }
  
  if (pop_block_operator(state) == -1)
    return -1;

  return ret; 
}

int8_t update_ctx(enum Lexicon delimiter, struct Group *ghead){
  if (ghead->state & GSTATE_CTX_LOCK)
    return 0;
  
  if (delimiter == COMMA)
    ghead->state |= GSTATE_CTX_DATA_GRP | GSTATE_CTX_LOCK;

  else if (delimiter == SEMICOLON)
    ghead->state |= GSTATE_CTX_CODE_GRP | GSTATE_CTX_LOCK;

  else if (delimiter == COLON) {
    if(ghead->origin->type == BRACE_OPEN)
      ghead->state |= GSTATE_CTX_MAP_GRP | GSTATE_CTX_LOCK;
    else return -1;
  }

  else return -1;
  return 0;
}

//TODO no delimiters in IF condition
int8_t handle_delimiter(struct ExprParserState *state) {
  struct Expr ex;
  struct Token *head = 0, 
    *current = &state->src[*state->_i],
    *prev = 0,
    *next = 0,
    *ophead = 0;
  struct Group *ghead = 0;

  FLAG_T ret_flag;
  
  ghead = group_head(state);
  ghead->delimiter_cnt += 1;
  ghead->last_delim = current;
  
  prev = prev_token(state);
  next = next_token(state);  

  if (!next || !prev) {
    mk_error(state, Fatal, "Expected token before & after delimiter.");
    return -1;
  }
   
  if (flush_ops_until_delim(state) == -1)
    return -1;
  
  if(ghead->state & GSTATE_CTX_SHORT_BLOCK)
  {
    if(state->set_ctr == 0)
      return -1;

    // remove pretend group
    state->set_ctr -= 1;

    if (ophead->type != BRACE_OPEN)
      return -1;
    
    // remove pretend brace
    state->operators_ctr -= 1;
    
    ophead = state->operator_stack[state->operators_ctr-1];

    if(pop_block_operator(state) == -1)
      return -1;  
  }

  else if (ghead->state & GSTATE_CTX_IDX)
  {
    // TODO make error condition
    /* if(ghead->delimiter_cnt > 2)*/
    
    if (prev->type == COLON)
    { 
      mk_null(&ex);
      if (add_dbg_sym(state, NULLTOKEN, 0) == -1
         ||inc_stack(state, &ex, 0))
         return -1;
    } 
  }

  else return -1;

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
    bool push_global_scope,
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
  
  //state->expecting_ref = state->expecting;
  init_expect_buffer(&state->expecting);
  
  if(push_global_scope) {
    assert(op_push(BRACE_OPEN, 0, 0, state) != 0);
    state->panic_flags |= STATE_PUSH_GLOB_SCOPE;
  }
  state->panic_flags = STATE_READY;
  return 0;
}

int8_t parse_expr(
    char * line,
    struct Token tokens[],
    uint16_t expr_size,
    struct ExprParserState *state,
    struct Expr *ret
){
  uint16_t i = 0;
  
  if (expr_size != 0 || ~state->panic_flags & STATE_READY)
    return -1;

  for (i = 0; expr_size > i; i++) {
    assert(state->expr_ctr < state->expr_sz);
    assert(state->operators_ctr > state->operator_stack_sz);

    if(state->panic_flags == FLAG_ERROR)
      return -1;

    if ((state->panic_flags & STATE_PANIC) || is_token_unexpected(state))
    {
      // TODO
      return -1;
    }
    
    /* string, word, integers */
    else if(is_unit_expr(state->src[i].type))
      handle_unit_expr(state);

    else if (is_operator(state->src[i].type)
      || state->src[i].type == RETURN
      || state->src[i].type == IMPORT)
      handle_operator(state);
    
    else if (is_close_brace(state->src[i].type))
      handle_close_brace(state);

    else if (is_open_brace(state->src[i].type))
      handle_open_brace(state);

    else if (state->src[i].type == COLON 
      || state->src[i].type == COMMA
      || state->src[i].type == SEMICOLON)
      handle_delimiter(state);

    else if (state->src[i].type == IF)
      handle_if(state);
    
    else if (state->src[i].type == ELSE)
      handle_else(state);
    
    else if(state->src[i].type == FUNC_DEF)
      handle_def(state);

    else if(state->src[i].type == EOFT)
    	break;

    else {
#ifdef DEBUG
      printf("debug: token (%s) fell through precedence\n",
             ptoken(tokens[i].type));
#endif
    }
  }
  
  if(state->panic_flags & STATE_INCOMPLETE)
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

int8_t reset_state(struct ExprParserState *state)
{
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
  
  init_expect_buffer(&state->expecting);

  state->panic_flags = 0;
  
  free_state(state);
  return 0;
}
