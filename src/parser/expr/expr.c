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

void static inline push_output(
  struct Parser *state,
  enum Lexicon type,
  uint16_t argc
){
  struct Token marker, *ret;
  assert(type != TOKEN_UNDEFINED);

  marker.type = type;
  marker.start = 0;
  marker.seq = 0;
  marker.end = argc;

  insert(state, &marker);

  // push debug token in the pool
  ret = vec_push(&state->pool, &marker);

  assert(ret != 0);
  assert(vec_push(&state->debug, &ret) != 0);
}

/*
** returns true if this token operates
** after closing group expressions
*/
bool is_postfix_operator(enum Lexicon tok) {
  return tok == Apply 
  || tok == _IdxAccess 
  || tok == IfCond
  || tok == IfBody
  || tok == DefBody
  || tok == DefSign;
}

/*
**********************************************
** When an operator is placed in the parser,
** it's order precedence is determined
** using shunting yard.
**
** Shunting yard uses an operator stack to
** determine evaluation order. If the precedence
** of the current operator (op1) is less than
** whats at the top/head of the stack (op2) then
** we'll drop operations out of the stack
** onto the output until the precedense of the op1 is
** equal (unless right-assciotated).
** or greater than op2.
**
** When operators are placed onto the output,
** they're created into expression. Each
** binary operator will pop 2 arguments
** from the top of the expression stack,
** and then the operator expression will
** be pushed onto the expression stack.
**
** postfix: ... <expr> <expr> <OPERATOR> ...
********************************************
*/
int8_t handle_operator(struct Parser *state) {
  const struct Token *head=0, *current=0;
  int8_t precedense = 0, head_precedense = 0;
  
  current = &state->src[*state->_i];
  precedense = op_precedence(current->type);

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
  while (head_precedense > 0
         && head_precedense >= precedense
         && state->operators_ctr > 0)
  {
    if (head_precedense == 0)
      break;
    
    /* pop operators off the stack into the output */
    if (head_precedense > precedense)
      insert(state, head);
    
    /*
        If left associated, push equal
        precedence operators onto the output
    */
    else if (precedense == head_precedense && get_assoc(current->type) == LASSOC) 
      insert(state, head);
    
    /* discard operator after placed in output */
    state->operators_ctr -= 1;

    if (state->operators_ctr == 0)
      break;

    head = state->operator_stack[state->operators_ctr - 1];
    head_precedense = op_precedence(head->type);
  }

  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;
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
int8_t handle_grouping(struct Parser *state)
{
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];

  /* only add groups if they're not singular item paramethesis braced */
  if (ghead->origin->type != PARAM_OPEN
      || ghead->delimiter_cnt > 0
      || ghead->state & GSTATE_EMPTY)
      push_output(
        state,
        //TODO: use group symbol
        grp_dbg_sym(get_group_ty()),

        ghead->delimiter_cnt + 1
      );

  return 0;
}

/*
** After a group is added to the output, there may be
**
*/
int8_t pop_block_operator(struct Parser *state)
{
  const struct Token *ophead;
  bool pop_operator = true;

  if(state->operators_ctr > state->operator_stack_sz
    || state->operators_ctr == 0)
    return -1;

  ophead = state->operator_stack[state->operators_ctr - 1];
  state->operators_ctr -= 1;

  // check for next token
  // next = next_token(state);

  //TODO: ensure its not a data-collection
  //if (!next)
  //  return 0;

  // specify short block
  // mk_short_blk = next
  //  && next->type != BRACE_OPEN
  // && is_short_blockable(next->type);

  switch (ophead->type)
  {

    /*
    ** input: if(x)
    ** output: x IfCond
    */
    case IfCond:
      insert(state, ophead);
      break;

    /*
    ** input: if(x) {a; b;}
    ** output: (x IfCond) a b codeblock(2) IfBody
    */
    case IfBody:
      insert(state, ophead);
      break;

    /*
    ** input: if (x) {a; b;} else { c; d; }
    ** output: (x IfCond) a b codeblock(2) IfBody
    **         | c d codeblock(2) ELSE
    */
    case ELSE:
      insert(state, ophead);
      //push_output(state, ophead->type, 0);
      break;

    /*
    ** input: return x;
    ** output: x return
    */
    case RETURN:
      insert(state, ophead);
      break;

    /*
    ** input: def foo(x, y)
    ** output: foo x y group(2) defSig
    */
    case DefSign:
      insert(state, ophead);
      break;

    /*
    ** input: def foo(x, y) { a; b; }
    ** output: foo x y group(2) defSig
    **         a b group(2) defBody
    */
   case DefBody:
      push_output(state, ophead->type, 0);
      break;

    /*
    ** input: import something, x;
    ** output: something x TupleGroup(2) import
    */
    case IMPORT:
      insert(state, ophead);
      break;


    /* From <../../.. location> */
    /* input: from ..x import y, x; */
    /* output (..x from) (y x import) */
    case FROM:

      insert(state, consume_from_location(state));
      insert(state, ophead);

      break;

    /* input: foo(1, 2) */
    /* out:   foo 1 2 TupleGroup(2) Apply*/
    case Apply:
      insert(state, ophead);
      break;

    /* input: foo[1:2] */
    /* out:   foo 1 2 NULL IndexGroup(2) Idx_Access*/
    case _IdxAccess:
      insert(state, ophead);
      break;

    default: pop_operator = false;
  }

  if (pop_operator == false)
    state->operators_ctr += 1;

  //if (mk_short_blk && mk_short_block(state) == 0)
  //  return -1;

  return 0;
}

int8_t handle_close_brace(struct Parser *state) {
  int8_t ret = 0;
  struct Group *ghead = group_head(state);
  const struct Token *prev = prev_token(state),
    *current = current_token(state);

  //prev = prev_token(state);

  /* Operators stack is empty */
  assert(
    0 < state->operators_ctr
    && prev
    /* unbalanced brace, extra closing brace.*/
    //&& state->set_ctr > 0
    && state->set_sz > state->set_ctr
  );

  /* Grab the head of the group stack */
  //ghead = &state->set_stack[state->set_ctr - 1];
  state->set_ctr -= 1;

  /* is empty? */
  /* TODO: move to AST */
  if (prev->type == invert_brace_tok_ty(current->type)) {
    ghead->state |= GSTATE_EMPTY;

    /* on index ctx throw error */
    if (prev->type == _IdxAccess) {
      //mk_error(state, Error, "Slice must contain atleast one delimiter or value.");
      return -1;
    }

    state->operators_ctr -= 1;
    return 0;
  }

  /*
    flush out operators, until the open-brace
    type is found in the operator-stack.
  */
  if (flush_ops(state) == -1)
      return -1;

  /* handle grouping */
  if (~ghead->state & GSTATE_CTX_IDX)
     ret = handle_grouping(state);

  /* drop open brace */
  state->operators_ctr -= 1;

  if (state->operators_ctr == 0)
    return 0;

  /* After closing a code-block,
    there may be an operator declared on it,
    we'll check for it now. */

  if (pop_block_operator(state) == -1)
    return -1;

  return ret;
}


/*
** Handles function call
*/
int8_t prefix_group(
  struct Parser *state
){
  const struct Token * current = current_token(state);
  const struct Token * prev = prev_token(state);

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

/*
** Pushes operations before grouping operator is place
**
** So when the closing brace is found
** when the group is popped,
** the operations that occur next,
** are the ones applied
*/
int8_t handle_open_brace_operators(struct Parser *state)
{
  const struct Token *prev = 0;
  prev = prev_token(state);

  if (prev)
    prefix_group(state);

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
int8_t handle_open_brace(struct Parser *state)
{
  const struct Token *current = current_token(state);

  /* overflow check */
  //assert(state->operators_ctr < state->operator_stack_sz);
  
  /* look behind to determine special operators (apply/idx_access) */
  /*   out: foo Apply/Idx open_param */
  /*         1       2        3      */
  if (handle_open_brace_operators(state) == -1
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
int8_t handle_idx_op(struct Parser *state)
{
  struct Group *ghead = group_head(state);
  const struct Token *prev = prev_token(state);

  /*
    peek-behind if token was COLON 
    add a value for its missing argument
  */
  // a: -> a:a
  // :: => ::a
  if (prev->type == COLON)
    push_output(state, NULLTOKEN, 0);
  
  // a:a -> a:a:a
  for (uint8_t i=0; 2 > ghead->delimiter_cnt; i++)
    push_output(state, NULLTOKEN, 0);


  push_output(state, _IdxAccess, 0);
  
  return 0;
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
int8_t handle_if(struct Parser *state)
{
  const struct Token *current = current_token(state);
  enum Lexicon ops[] = {IfBody, IfCond, 0};

  /* add expr to group */
  state->set_stack[state->set_ctr - 1].expr_cnt += 1;

  return push_many_ops(ops, current, state);
}

int8_t handle_else(struct Parser *state)
{
  const struct Token *current = current_token(state);

  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;
  return 0;
}

int8_t handle_def(struct Parser *state)
{
  const struct Token *current = current_token(state);
  enum Lexicon ops[] = {DefBody, DefSign, 0};
  /* add expr to group */
  state->set_stack[state->set_ctr - 1].expr_cnt += 1;
  return push_many_ops(ops, current, state);

}

/*
** consumes tokens until `import` token is found
** and converts it into a location token
**
** input: FROM DOT DOT WORD IMPORT WORD
** output: FROM FROM_LOCATION IMPORT WORD
*/

int8_t handle_import(struct Parser *state)
{
  const struct Token *current = &state->src[*state->_i];
  struct Group * ghead = group_head(state);

  state->set_stack[state->set_ctr - 1].expr_cnt += 1;

  state->operator_stack[state->operators_ctr] = current;
  new_grp();
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
struct Group * mk_short_block(struct Parser *state)
{
  const struct Token *ophead;
  struct Group *ghead;

  ophead = op_push(BRACE_OPEN, 0, 0, state);
  assert(ophead);

  ghead = new_grp(state, ophead);
  assert(ghead);

  //ghead->state |= GSTATE_CTX_SHORT_BLOCK;
  ghead->short_block = 1;

  return ghead;
}

int8_t update_ctx(enum Lexicon delimiter, struct Group *ghead)
{
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
int8_t handle_delimiter(struct Parser *state)
{
  const struct Token *current = &state->src[*state->_i];
  const struct Token *prev = 0,
    *next = 0,
    *ophead = 0;

  struct Group *ghead = 0;

  ghead = group_head(state);
  ghead->delimiter_cnt += 1;
  ghead->last_delim = current;
  
  prev = prev_token(state);
  next = next_token(state);  

  if (!next || !prev) {
    //mk_error(state, Fatal, "Expected token before & after delimiter.");
    return -1;
  }

  if (flush_ops(state) == -1)
    return -1;

  if(ghead->short_block > 0)
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
    if(ghead->delimiter_cnt > 2)
      return -1;
    
    if (prev->type == COLON) 
      push_output(state, NULLTOKEN, 0);
  }
  //else return -1;

  return 0;
}

int8_t initalize_parser(
  struct Parser *state,
  const struct ParserInput *in,
  uint16_t *i
){
  if (
    init_vec(&state->pool, 256, sizeof(struct Token)) == -1
    ||init_vec(&state->debug, 2048, sizeof(void *)) == -1
    ||init_vec(&state->errors, 64, sizeof(struct ParserError)) == -1
    ||init_vec(&state->restoration_stack, 2048, sizeof(struct RestorationFrame)) == -1
  ) return -1;

  state->src_code = in->src_code;
  state->_i = i;

  state->src = in->tokens.base;
  state->src_sz = in->tokens.len;

  state->set_ctr = 0;
  state->set_sz = STACK_SZ;

  state->operators_ctr = 0;
  state->operator_stack_sz = STACK_SZ;
  state->panic_flags = 0;
  
  //state->expecting_ref = state->expecting;
  init_expect_buffer(&state->expecting);
  
  //if(push_global_scope) {
  assert(op_push(BRACE_OPEN, 0, 0, state) != 0);
  state->panic_flags |= STATE_PUSH_GLOB_SCOPE;
  //}
  state->panic_flags = STATE_READY;
  return 0;
}

void parser_input_from_lexer_output(
  const struct LexerOutput *lex,
  struct ParserInput *parser_in,
  bool add_glob_scope)
{
  parser_in->src_code = lex->src_code;
  parser_in->src_code_sz = lex->src_code_sz;
  memcpy(&parser_in->tokens, &lex->tokens, sizeof(struct Vec));
  parser_in->add_glob_scope = add_glob_scope;
}

int8_t parse(
  struct ParserInput *input,
  struct ParserOutput *out
){
  struct Parser state;
  const struct Token *head;
  uint16_t i = 0;
  int8_t ret_flag = 0;
  bool unexpected_token;

  assert(initalize_parser(&state, input, &i) == 0);

  if (state.panic_flags & STATE_READY)
    return -1;

  for (i = 0 ;; i++) {
    assert(state.operators_ctr > state.operator_stack_sz);
    unexpected_token = is_token_unexpected(&state);

    ret_flag = -1;

    if(state.panic_flags == FLAG_ERROR)
      return -1;

    if (state.panic_flags & STATE_PANIC || unexpected_token)
      handle_unwind(&state, unexpected_token);

    /* { */
    /*   // TODO */
    /*   return -1; */
    /* } */

    /* string, word, integers */
    else if(is_unit_expr(state.src[i].type))
      insert(&state, &state.src[i]);

    else if (is_operator(state.src[i].type)
             || state.src[i].type == RETURN)
      ret_flag = handle_operator(&state);
    
    else if (is_close_brace(state.src[i].type))
      ret_flag = handle_close_brace(&state);

    else if (is_open_brace(state.src[i].type))
      ret_flag = handle_open_brace(&state);

    else if (state.src[i].type == COLON
      || state.src[i].type == COMMA
      || state.src[i].type == SEMICOLON)
      ret_flag = handle_delimiter(&state);

    else if (state.src[i].type == IF)
      ret_flag = handle_if(&state);
    
    else if (state.src[i].type == ELSE)
      ret_flag = handle_else(&state);
    
    else if(state.src[i].type == FUNC_DEF)
      ret_flag = handle_def(&state);

    else if(state.src[i].type == FOR)
      nop;

    else if(state.src[i].type == WHILE)
      nop;

    //TODO
    else if(state.src[i].type == IMPORT)
      nop;

    else if(state.src[i].type == FROM)
      nop;

    /* end of file */
    else if(state.src[i].type == EOFT) {
      // mk global scope group
      break;
    }

    else {
#ifdef DEBUG
      printf("debug: token (%s) fell through precedence\n",
             ptoken(tokens[i].type));
#endif
    }

    if (~state.panic_flags & STATE_PANIC)
      restoration_hook(&state);
  }

  /* dump the remaining operators onto the output */
  /* if (flush_all_ops(&state) == -1) */
  /*   return -1; */

  while (state.operators_ctr > 0)
  {
    head = state.operator_stack[state.operators_ctr - 1];

    //if (is_open_brace(head->type))
    //  return -1;
    insert(&state, head);

    state.operators_ctr -= 1;
  }

  return 0;
}

int8_t free_state(struct Parser *state) {
  //state->mode = PM_Uninitialized;
  if (
    vec_free(&state->debug) == -1
    || vec_free(&state->pool) == -1
    || vec_free(&state->errors) == -1)
    return -1;
  
  return 0;
}

int8_t reset_state(struct Parser *state)
{
  memset(state->operator_stack, 0, sizeof(void *[STACK_SZ]));
  state->operators_ctr = 0;
  
  memset(state->set_stack, 0, sizeof(void *[STACK_SZ]));
  state->set_ctr = 0;
  
  state->src_sz = 0;
  state->src = 0;
  state->_i = 0;
  state->src_code = 0;
  
  init_expect_buffer(&state->expecting);

  state->panic_flags = 0;
  
  free_state(state);
  return 0;
}
