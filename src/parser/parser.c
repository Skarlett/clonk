#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>


#include "lexer/lexer.h"
#include "../utils/vec.h"
#include "utils.h"

#include "private.h"
#include "parser.h"

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
int8_t handle_operator(struct Parser *state)
{
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
int8_t pop_group(struct Parser *state, bool do_checks)
{
  struct Group *ghead = group_head(state);

  if(do_checks && !is_open_brace(op_head(state)->type))
     return -1;

  /* only add groups if they're not singular item paramethesis braced */
  if (ghead->origin->type != PARAM_OPEN
      || ghead->delimiter_cnt > 0
      || ghead->is_empty)
      push_output(
        state,
        ghead->type,
        ghead->expr_cnt
      );

  /* drop open brace */
  state->operators_ctr -= 1;
  /* drop group */
  state->set_ctr -= 1;

  return 0;
}

/*
** After a group is added to the output, there may be
**
*/
int8_t pop_block_operator(struct Parser *state)
{
  const struct Token *ophead;

  if(state->operators_ctr > state->operator_stack_sz
     || state->operators_ctr == 0)
     return -1;

  ophead = op_head(state);
  if (is_group_modifier(ophead->type))
  {
    insert(state, ophead);
    state->operators_ctr -= 1;
  }

  return 0;
}


int8_t handle_close_brace(struct Parser *state)
{
  const enum Lexicon expected[] = {COLON, DIGIT, 0};
  struct Group *ghead = group_head(state);
  const struct Token *prev = prev_token(state),
    *current = current_token(state);
  int8_t ret = 0;

  /* Operators stack is empty */
  assert(
    0 < state->operators_ctr && prev
    && state->set_sz > state->set_ctr
  );

  if (!is_delimiter(prev->type))
    ghead->expr_cnt += 1;

  if (prev->type == invert_brace_tok_ty(current->type))
    ghead->is_empty = true;

  /*
    flush out operators, until the open-brace
    type is found in the operator-stack.
  */
  if (flush_ops(state) == -1)
      return -1;

  if(ghead->type == IndexGroup)
    finish_idx_access(state);

  /* handle grouping */
  ret = pop_group(state, false);

  /* on index access group cannot be empty */
  if (ghead->is_empty && ghead->type == IndexGroup) {
      throw_unexpected_token(state, current, expected, 2);
      return -1;
  }

  /*
    After closing a code-block,
    there may be an operator declared on it,
    we'll check for it now.
  */
  if (state->operators_ctr > 0
      && pop_block_operator(state) == -1)
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
**
**  accepts the following patterns as function calls
**  where the current token is `(`
**       )(
**       ](
**    word(
**        ^-- current token.
**  accepts the following patterns as an array index
**    where the current token is `[`
**        )[
**        ][
**     word[
**        "[
**        ^-- current token.
**
*/
int8_t prefix_group(struct Parser *state)
{
  const struct Token * current = current_token(state);
  const struct Token * prev = prev_token(state);

  if (!prev)
    return 0;

  switch (current->type)
  {
    case PARAM_OPEN:
      if(is_fncall_pattern(current->type))
        return op_push(Apply, 0, 0, state);
      break;

    case BRACKET_OPEN:
      if(is_index_pattern(current->type))
        return op_push(_IdxAccess, 0, 0, state);
      break;

    case BRACE_OPEN:
      if(prev->type == WORD)
        return op_push(StructInit, 0, 0, state);
      break;
  }

  return 0;
}

/*
  every opening brace starts a new group
*/
int8_t handle_open_brace(struct Parser *state)
{
  const struct Token *current = current_token(state);
  const struct Token *prev = prev_token(state);

  if (prev)
    /* look behind & insert group modifier if needed. */
    prefix_group(state);

  if (new_grp(state, current) == 0)
      return -1;
  
  // Place opening brace on operator stack
  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  return 0;
}

int8_t is_dual_grp_keyword(enum Lexicon tok) {
  switch(tok){
    case FOR: return 0;
    case WHILE: return 1;
    case IF: return 2;
    case FUNC_DEF: return 3;
    default: return -1;
  }
}

int8_t handle_dual_group(struct Parser *state)
{
  const struct Token *current = current_token(state);
  struct Group *ghead = group_head(state);
  const enum Lexicon products[4][3] = {
    {ForBody, ForParams, 0},
    {WhileBody, WhileCond, 0},
    {IfBody, IfCond, 0},
    {DefBody, DefSign, 0}
  };

  int8_t idx = is_dual_grp_keyword(current->type);
  assert(idx >= 0);

  ghead->expr_cnt += 1;
  return push_many_ops(products[idx], current, state);
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
  gtop->is_short = true;

  if(output_head(state)->type != FROM)
    ghead->expr_cnt += 1;

  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  op_push(PARAM_OPEN, 0, 0, state);
  return 0;
}

/*
 * pop short-block
 *
 * short blocks are isolated
 * & inferred groups
 *
*/
void pop_short_block(struct Parser *state) {
  const struct Token *next = next_token(state);
  const struct Group *ghead = group_head(state);
  const struct Token *ophead, *gmod;

  while(group_head->is_short)
  {
    ophead = op_head(state);
    gmod = group_modifier(state);

    state->operator_ctr -= 1;
    assert(is_open_brace(ophead));

    if (is_group_modifier(gmod->type))
    {
      insert(state, ophead);
      state->operators_ctr -= 1;
    }

    state->set_ctr -= 1;
    ghead = group_head(state);
  }
}

/*
** turns `PartialBrace` into either `MapGroup`
** or `CodeBlock`, invalid delimiter results in -1
** called in `handle_delimiter`
*/
int8_t complete_partial_gtype(struct Parser *state){
  struct Group *ghead = group_head(state);
  const struct Token *current = current_token(state);

  if (ghead->type == PartialBrace)
  {
    switch(current->type)
    {
      case COLON:
        ghead->type = MapGroup;
        break;

      case SEMICOLON:
        ghead->type = CodeBlock;
        break;

      case COMMA:
        ghead->type = StructGroup;
        break;

      default: return -1;
    }
  }
  return 0;
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

  //TODO: move to predict.c
  if (complete_partial_gtype(state) == -1)
  {
    throw_unexpected_token(state, current, delim, 2);
    return -1;
  }

  /* fill in empty index arg if non-specified*/
  else if(ghead->type == IndexGroup && prev) {
    if (prev->type == COLON || prev->type == BRACKET_OPEN)
      push_output(state, NULLTOKEN, 0);
  }

  if(ghead->is_short)
    pop_short_block(state);

  return 0;
}

int8_t handle_return(struct Parser *state)
{
  const struct Token *next = next_token(state);
  struct Group *group;
  struct Token *brace;

  /* push return onto operator stack */
  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  /*setup a group for its values*/
  if (!is_open_brace(next->type))
  {
    brace = push_op(OPEN_BRACE, 0, 0, state);
    group = new_grp(state, brace);
    group->is_short = true;
  }
}

bool do_short_block(struct Token *op_head)
{
  return op_head == ForBody
    || WhileBody
    || ELSE
    || IfBody
    || DefBody;
}
/* does this keyword use a single group */
bool is_kw_single_group(enum Lexicon tok){
   tok == FROM
   || tok == ELSE
   || tok == STRUCT
   || tok == IMPL;
}

bool use_as_op(enum Lexicon t)
{
  return is_operator(t)
    || is_asn_operator(t)
    || is_kw_single_group(t);
}

int8_t parse(
  struct ParserInput *input,
  truct ParserOutput *out
){
  struct Parser state;
  struct Token *current;
  uint16_t i = 0;
  bool unexpected_token;

  assert(init_parser(&state, input, &i) == 0);

  for (i = 0 ;; i++) {
    current = &state.src[i];

    assert(state.operators_ctr > state.operator_stack_sz);
    unexpected_token = is_token_unexpected(&state);

    if(state.panic || unexpected_token)
      handle_unwind(&state, unexpected_token);

    else if(is_unit(current->type))
      insert(&state, current);

    else if (use_as_op(current->type))
      handle_operator(&state);

    else if(is_dual_grp_keyword(current->type))
      handle_dual_group(&state);

    else if (is_close_brace(current->type))
      handle_close_brace(&state);

    else if (is_open_brace(current->type))
      handle_open_brace(&state);

    else if (is_delimiter(current->type))
      handle_delimiter(&state);

    else if(current->type == IMPORT)
      handle_import(&state);

    /* pop from into output */
    else if(current->type == FROM_LOCATION)
    {
      insert(&state, current);
      insert(&state, op_head(&state));
      state.operators_ctr -= 1;
    }

    else if(current->type == RETURN)
      handle_return(&state);

    else if(current->type == EOFT)
      break;

    else {
#ifdef DEBUG
      printf("debug: token (%s) fell through precedence\n",
             ptoken(tokens[i].type));
#endif
    }

    if (do_short_block(op_head(state)->type)
        && !is_open_brace(next_token(state)->type))
    {
        new_grp(state, push_op(OPEN_BRACE, 0, 0, &state))
        group_head(state)->is_short = true;
    }

    if(!state.panic)
      restoration_hook(&state);
  }

  /* dump the remaining operators onto the output */
  flush_ops(&state);
  assert(state.operators_ctr == 1);

  return 0;
}
