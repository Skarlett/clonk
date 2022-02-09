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

bool is_dual_grp_keyword(enum Lexicon tok) {
  return tok == FOR
    || tok == WHILE
    || tok == IF
    || tok == FUNC_DEF;
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

void push_output(
  struct Parser *state,
  enum Lexicon type,
  uint16_t argc
){
  struct Token marker;
  const struct Token *ret;
  assert(type != TOKEN_UNDEFINED);


  //TODO: double check sequence unwinding
  marker.seq = 0;

  marker.type = type;
  marker.start = 0;
  marker.end = argc;

  ret = new_token(state, &marker);

  assert(ret);
  insert(state, ret);
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
  int8_t precedense = 0, head_precedense = 0;
  const struct Token *current = current_token(state),
    *head = 0;
  
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
  head = op_head(state);
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
 * dbg: <body-expr> ... Group_t
 */
int8_t pop_group(struct Parser *state)
{
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];

  /* only add groups if they're not singular item paramethesis braced */
  if (ghead->origin->type != PARAM_OPEN
      || ghead->delimiter_cnt > 0
      || ghead->state & GSTATE_EMPTY)

  push_output(
    state,
    ghead->type,

    /* TODO: Better expression counting */
    /* NOTE: current code probably barely works */
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

  ophead = op_head(state);

  if (is_group_modifier(ophead->type))
  {
    insert(state, ophead);
    state->operators_ctr -= 1;
  }

  // check for next token
  // next = next_token(state);

  //TODO(shortblocks): ensure its not a data-collection
  //if (!next)
  //  return 0;

  // specify short block
  // mk_short_blk = next
  //  && next->type != BRACE_OPEN
  // && is_short_blockable(next->type);


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
    0 < state->operators_ctr && prev
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
** Handles group notation operations such as
** function call (`foo(x)`) & index access (`foo[x]`)
**
** Places `Apply` before the open-brace type `(`
** when the opposite brace type is found,
** and the group is put into the output,
** `pop_block_operators` will be ran,
** and insert the token before it (`Apply`/`IndexAccess`)
** if that token is defined as a block-descriptor.
*/
int8_t prefix_group(
  struct Parser *state
){
  const struct Token * current = current_token(state);
  const struct Token * prev = prev_token(state);

  bool pushed = 0;

  /* function call pattern */
  if (current->type == PARAM_OPEN
    && (is_close_brace(prev->type) || prev->type == WORD)
    && (pushed = (op_push(Apply, 0, 0, state) == 0)))
    return -1;

  /* index call pattern */
  else if (current->type == BRACKET_OPEN)
  {
    /* is indexable */
    if ((is_close_brace(prev->type)
       || prev->type == WORD
       || prev->type == STRING_LITERAL)
       && (pushed = op_push(_IdxAccess, 0, 0, state) == 0))
       return -1;
  }

  return pushed;
}

/*
  every opening brace starts a new group
*/
int8_t handle_open_brace(struct Parser *state)
{
  bool ret;
  const struct Token *current = current_token(state);
  const struct Token *prev = prev_token(state);

  if (prev)
    /* look behind & insert group modifier if needed. */
    ret = prefix_group(state);

  if (new_grp(state, current) == 0)
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

int8_t handle_dual_group(struct Parser *state, uint16_t id)
{
  struct Group *ghead = group_head(state);
  static enum Lexicon products[][3] = {
    {ForBody, ForParams, 0},
    {WhileBody, WhileCond, 0},
    {IfBody, IfCond, 0},
    {DefBody, DefSign, 0},
    {0, 0, 0}
  };

  ghead->expr_cnt += 1;
  return push_many_ops(products[id], current_token(state), state);
}

/*
** consumes tokens until `import` token is found
** and converts it into a location token
**
** input: from ..word import new_word;
** output: ..word from new_word g(1) import
**
** input: import word;
** output word g(1) import
*/
int8_t handle_import(struct Parser *state)
{
  struct Group *gtop, *ghead = group_head(state);
  const struct Token *current = current_token(state);

  gtop = new_grp(state, next_token(state));
  gtop->short_type = sh_import_t;

  if(output_head(state)->type != FROM)
    ghead->expr_cnt += 1;

  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  op_push(PARAM_OPEN, 0, 0, state);
  return 0;
}

int8_t handle_sb_cond_termination(struct Parser *state)
{

  /* if(x)
  **   if(b) c;
  **   else d;
  **
  */
  if (next_token(state)->type == ELSE)
    state->operators_ctr -= 1;
  else {
    // TODO:
    // implemented chained conditional short-blocks
  }
}


int8_t handle_sb_import(struct Parser *state) {

  const struct Group *ghead = group_head(state);

  /* pop off the group */
  state->set_ctr -= 1;
  /* remove open_param in operators */
  state->operators_ctr -= 1;

  push_group(state, ghead);

}

int8_t handle_short_block_termination(struct Parser *state) {
  const struct Group *ghead = group_head(state);

  if (ghead->short_type == sh_import_t)
    handle_sb_import(state);

  else if(ghead->short_type == sh_cond_t)
    handle_sb_cond_termination(state);
}


/*
** turns `PartialBrace` into either `MapGroup`
** or `CodeBlock`, invalid delimiter results in -1
** called in `handle_delimiter`
*/

int8_t _complete_partial_gtype(struct Group * ghead, const struct Token *current){
    if (current->type == COLON)
      ghead->type = MapGroup;
    else if(current->type == SEMICOLON)
      ghead->type = CodeBlock;
    else
      return -1;
    return 0;
}

int8_t complete_partial_gtype(struct Parser *state){
  struct Group *ghead = group_head(state);
  const struct Token *current = current_token(state);

  if (ghead->type == PartialBrace)
    return _complete_partial_gtype(ghead, current);
  return 0;
}

/* throw unexpected token */
int8_t is_illegal_delimiter(const struct Parser *state) {
  struct Group *ghead = group_head(state);
  const struct Token *next = next_token(state);
  const struct Token *gmod = group_modifier(state, ghead);


  //TODO: add to predict.c
  //if (ghead->type != CodeBlock)
  //{
  //  if(is_delimiter(next->type))
  //    return -1;
  //}

  return \
    gmod->type == WhileCond
    || gmod->type == IfCond
    || (ghead->type == _IdxAccess && ghead->delimiter_cnt > 2)
    /* check for correct delimiter*/
    // TODO: Handle in predict.c
    //|| (gmod->type == _IdxAccess && current->type != COLON)
    //|| ((ghead->type == TupleGroup || ghead->type == ListGroup) && current->type != COMMA)
    //|| (ghead->type == CodeBlock && current->type != SEMICOLON)
    ;
}

int8_t handle_delimiter(struct Parser *state)
{
  const enum Lexicon delim[2] = {COLON, SEMICOLON};
  struct Group *ghead = group_head(state);
  const struct Token
    *current = current_token(state),
    *prev = prev_token(state);

  ghead->delimiter_cnt += 1;
  ghead->last_delim = current;

  flush_ops(state);

  // TODO: see how this plays with if/for/while
  if (!is_delimiter(prev->type))
    ghead->expr_cnt += 1;

  if (is_illegal_delimiter(state) || complete_partial_gtype(state) == -1)
  {
    throw_unexpected_token(state, current, delim, 2);
    return -1;
  }

  /* fill in empty index arg if non-specified*/
  else if(ghead->type == IndexGroup && prev && prev->type == COLON)
    push_output(state, NULLTOKEN, 0);

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

  //state->expecting_ref = state->expecting;
  init_expect_buffer(&state->expecting);
  
  assert(op_push(BRACE_OPEN, 0, 0, state) != 0);
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
  uint16_t i = 0;
  int8_t ret_flag = 0;
  int8_t ez_match_id = 0;
  bool unexpected_token;

  assert(initalize_parser(&state, input, &i) == 0);

  for (i = 0 ;; i++) {
    assert(state.operators_ctr > state.operator_stack_sz);
    unexpected_token = is_token_unexpected(&state);

    if(state.panic || unexpected_token)
      handle_unwind(&state, unexpected_token);

    if(is_dual_grp_keyword(state.src[i].type))
      handle_dual_group(&state, ez_match_id);

    /* string, word, integers */
    else if(is_unit(state.src[i].type))
      insert(&state, &state.src[i]);

    else if (is_operator(state.src[i].type)
             || state.src[i].type == RETURN
             || state.src[i].type == FROM
             || state.src[i].type == ELSE)
      ret_flag = handle_operator(&state);
    
    else if (is_close_brace(state.src[i].type))
      ret_flag = handle_close_brace(&state);

    else if (is_open_brace(state.src[i].type))
      ret_flag = handle_open_brace(&state);

    else if (is_delimiter(state.src[i].type))
      ret_flag = handle_delimiter(&state);

    else if(state.src[i].type == IMPORT)
      handle_import(&state);

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

    if(!state.panic)
      restoration_hook(&state);
  }

  /* dump the remaining operators onto the output */
  flush_ops(&state);

  assert(state.operators_ctr == 1);

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

  //state->panic_flags = 0;
  
  free_state(state);
  return 0;
}
