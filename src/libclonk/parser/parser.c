#include "clonk.h"
#include "lexer.h"
#include "onkstd/vec.h"
#include "parser.h"
#include "private.h"
#include "semantics.h"
#include <stdint.h>

enum Associativity
{
    NONASSOC,
    RASSOC,
    LASSOC
};

enum Associativity get_assoc(enum onk_lexicon_t token)
{
  if(onk_is_tok_unary_operator(token)
    || token == ONK_POW_TOKEN)
    return RASSOC;
  return LASSOC;
}

bool is_token_unexpected(struct onk_parser_state_t*state)
{
  enum onk_lexicon_t current = current_token(state)->type;
  struct validator_frame_t frame;

  assert(state->nexpect > 0);

  if(onk_semantic_check(state, current))
    return true;

  frame.slices = state->exp_slices_buf;
  frame.islices = state->islices_buf;
  frame.nslices = 0;

  /* grab frame slices */
  onk_semantic_build_frame(state);

  /* compile into continguent array */
  state->nexpect = onk_semantic_compile(&frame, state);

  return false;
}

enum onk_lexicon_t group_type_init(enum onk_lexicon_t brace)
{
  switch (brace)
  {
    case ONK_PARAM_OPEN_TOKEN:
      return onk_tuple_group_token;

    case ONK_BRACKET_OPEN_TOKEN:
      return onk_list_group_token;

    case ONK_BRACE_OPEN_TOKEN:
      return onk_code_group_token;

    case ONK_HASHMAP_LITERAL_START_TOKEN:
      return onk_map_group_token;

    default:
      return ONK_UNDEFINED_TOKEN;
  }
}

void init_grp(
  struct onk_parse_group_t * ghead,
  const struct onk_token_t * from
){
  ghead->last_delim = 0;
  ghead->delimiter_cnt = 0;
  ghead->expr_cnt = 0;

  ghead->is_short = false;
  ghead->collapse = false;

  ghead->origin = from;
  ghead->type = group_type_init(from->type);

  assert(ghead->type != ONK_UNDEFINED_TOKEN);
}

/* NOTE: assumes `OPEN_BRACE/PARAM/BRACKET` is on the operator stack*/
struct onk_parse_group_t * new_grp(
  struct onk_parser_state_t* state,
  const struct onk_token_t * from
){
  struct onk_parse_group_t *ghead = 0;
  assert(state->set_ctr < state->set_sz);

  ghead = &state->set_stack[state->set_ctr];
  state->set_ctr += 1;

  /* NOTE: ASSUMPTION HERE */
  ghead->operator_idx = state->operators_ctr - 1;
  ghead->set_idx = state->set_ctr - 1;

  init_grp(ghead, from);
  return ghead;
}

void onk_parser_init(
  struct onk_parser_state_t *state,
  uint16_t *i
){
  state->set_ctr = 0;
  state->set_sz = ONK_STACK_SZ;
  state->operators_ctr = 0;
  state->operator_stack_sz = ONK_STACK_SZ;

  state->panic = false;
  state->_i = i;

  state->peek_next = 0;
  state->peek_prev = 0;

  state->_mode = onk_parser_init_mode;
}

void onk_parser_alloc(struct onk_parser_state_t * state)
{
  assert(state->_mode == onk_parser_init_mode);
  onk_vec_new(&state->pool, 256, sizeof(struct onk_token_t));
  onk_vec_new(&state->debug, 2048, sizeof(void *));
  onk_vec_new(&state->errors, 64, sizeof(struct onk_parser_err_t));
  onk_vec_new(&state->restoration_stack, 2048, sizeof(struct onk_parser_snapshot_t));
  state->_mode = onk_parser_alloc_mode;
}

/* Write `_EX_EXPR` into state->expecting */
void onk_semenatic_setup(struct onk_parser_state_t * state)
{
  enum onk_lexicon_t default_expected[] = {
    ONK_WHITESPACE_TOKEN, _EX_EXPR, _EX_KWORD_BLOCK
  };

  state->nexpect = EXPR_LEN + KWORD_BLOCK_LEN;
  assert(memcpy(state->expect, default_expected, KWORD_BLOCK_SZ + EXPR_SZ));
}

void onk_parser_setup(struct onk_parser_state_t *state){
    const struct onk_token_t *global_namespace = 0;
    struct onk_parse_group_t *ghead = &state->set_stack[0];

    assert(state->_mode == onk_parser_alloc_mode);
    onk_semenatic_setup(state);

    // Initalize global name space
    init_grp(ghead, (void *)global_namespace);
    ghead->operator_idx = 0;
    ghead->set_idx = 0;

    state->set_ctr = 1;
    state->operators_ctr = 1;

    state->_mode = onk_parser_awaiting_input_mode;
}

void onk_parser_setup_input(
    struct onk_parser_state_t *state,
    const struct onk_parser_input_t *input
){
    assert(state->_mode == onk_parser_awaiting_input_mode);

    state->tokens = input->tokens.base;
    state->token_len = input->tokens.len;
    state->_mode = onk_parser_ready_mode;
}

void onk_parser_construct(struct onk_parser_state_t *parser, uint16_t *i, struct onk_parser_input_t *input)
{
    onk_parser_init(parser, i);
    onk_parser_alloc(parser);
    onk_parser_setup(parser);
    onk_parser_setup_input(parser, input);
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
int8_t handle_operator(struct onk_parser_state_t*state)
{
  int8_t precedense = 0, head_precedense = 0;
  const struct onk_token_t *current = current_token(state),
    *head = 0;

  precedense = op_precedence(current->type);

  /*
    If no operators are in operators-stack,
    place it directly.

    If the head of the operator stack is an open brace
    we don't need to do anymore checks
    before placing the operator  
  */  
  if (precedense == -1)
      return -1;

  else if (state->operators_ctr == 0 || precedense == 0)
  {
    state->operator_stack[state->operators_ctr] = current;
    state->operators_ctr += 1;
    return 0;
  }

  /* Grab the head of the operators-stack */
  head = op_head(state);

  /* skip IN operator if apart of ForLoopBody */
  if(head->type == onk_for_body_op_token && current->type == ONK_IN_TOKEN)
    return 0;

  head_precedense = op_precedence(head->type);
  if (0 > head_precedense)
      return -1;
  /*
   * while `head` has higher precedence
   * than our current token pop operators from
   * the operator-stack into the output
  */
  while (head_precedense > 0
         && head_precedense >= precedense
         && state->operators_ctr > 0)
  {
    if (head_precedense == -1) return -1;
    else if (head_precedense == 0)
      break;
    
    /* pop operators off the stack into the output */
    else if (head_precedense > precedense)
      insert(state, head);
    
    /*
     *  If left associated, push equal
     *  precedence operators onto the output
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
 * N is derived from `struct onk_parse_group_t`'s delmiter_ctr + 1.
 *
 * Grouping represents sets of data like
 * lists, tuples, & codeblocks.
 *
 * src: [body_expr, ...]
 * dbg: <body-expr> ... Group_t
*/
int8_t pop_group(struct onk_parser_state_t *state, bool do_checks)
{
  struct onk_parse_group_t *ghead = group_head(state);

  if(do_checks && !onk_is_tok_open_brace(op_head(state)->type))
     return -1;

  /* only add groups if they're not singular item paramethesis braced */
  if (ghead->origin->type != ONK_PARAM_OPEN_TOKEN
      && ghead->delimiter_cnt != 1)
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
** group operation
*/
int8_t pop_block_operator(struct onk_parser_state_t*state)
{
  const struct onk_token_t *ophead = 0;

  if(state->operators_ctr > state->operator_stack_sz
     || state->operators_ctr == 0)
     return -1;

  ophead = op_head(state);

  if (onk_is_tok_group_modifier(ophead->type))
  {
    insert(state, ophead);
    state->operators_ctr -= 1;
  }

  return 0;
}


int8_t handle_close_brace(struct onk_parser_state_t *state)
{
  const enum onk_lexicon_t expected[] = {ONK_COLON_TOKEN, ONK_DIGIT_TOKEN, 0};
  struct onk_parse_group_t *ghead = group_head(state);
  const struct onk_token_t *prev = prev_token(state),
    *current = current_token(state);
  int8_t ret = 0;

  /* Operators stack is empty */
  assert(
    0 < state->operators_ctr && prev
    && state->set_sz > state->set_ctr
  );

  if (!onk_is_tok_delimiter(prev->type))
    ghead->expr_cnt += 1;

  if (prev->type == onk_invert_brace(current->type))
    ghead->is_empty = true;

  /*
    flush out operators, until the open-brace
    type is found in the operator-stack.
  */
  if (flush_ops(state) == -1)
      return -1;

  if(ghead->type == onk_idx_group_token)
    finish_idx_access(state);

  /* handle grouping */
  ret = pop_group(state, false);

  /*TODO: move to predict.c*/
  /* on index access group cannot be empty */
  if (ghead->is_empty && ghead->type == onk_idx_group_token) {
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
** `Init {a=b}`
** Places `onk_apply_op_token` before the open-brace type `(`
** when the opposite brace type is found,
** and the group is put into the output,
** `pop_block_operators` will be ran,
** and insert the token before it (`onk_apply_op_token`/`IndexAccess`)
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
**         ^-- current token.
**
**     word{
**         ^---current token.
**
*/
enum onk_lexicon_t push_group_modifier(
  const struct onk_token_t * current,
  const struct onk_token_t * prev
){
  if(!prev)
    return 0;

  switch (current->type)
  {
    /* foo(bar) */
    case ONK_PARAM_OPEN_TOKEN:
      if(is_fncall_pattern(prev->type))
        return onk_apply_op_token;
      break;

    /* data[index] */
    case ONK_BRACKET_OPEN_TOKEN:
      if(is_index_pattern(prev->type))
        return onk_idx_op_token;
      break;

    /* structure { word=val } */
    case ONK_BRACE_OPEN_TOKEN:
      if(prev->type == ONK_WORD_TOKEN)
        return onk_struct_init_op_token;
      break;

    default:
      return 0;
  }

  return 0;
}

/*
  every opening brace starts a new group
*/
int8_t handle_open_brace(struct onk_parser_state_t*state)
{
  const struct onk_token_t *current = current_token(state);
  const struct onk_token_t *prev = prev_token(state);
  enum onk_lexicon_t modifier = push_group_modifier(current, prev);

  // push apply/index
  if (prev && modifier)
    op_push(modifier, 0, 0, state);

  /* look behind & insert group modifier if needed. */
  if (new_grp(state, (void *)current) == 0)
      return -1;
  
  // Place opening brace on operator stack
  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  return 0;
}

int8_t is_dual_grp_keyword(enum onk_lexicon_t tok) {
  switch(tok){
    case ONK_FOR_TOKEN: return 0;
    case ONK_WHILE_TOKEN: return 1;
    case ONK_IF_TOKEN: return 2;
    case ONK_DEF_TOKEN: return 3;
    default: return -1;
  }
}

void handle_dual_group(struct onk_parser_state_t*state)
{
  const struct onk_token_t *current = current_token(state);
  const struct onk_token_t *next = next_token(state);
  struct onk_parse_group_t *ghead = group_head(state);
  const enum onk_lexicon_t products[4][3] = {
    {onk_for_body_op_token, onk_for_args_op_token, 0},
    {onk_while_body_op_token, onk_while_cond_op_token, 0},
    {onk_ifbody_op_token, onk_ifcond_op_token, 0},
    {onk_defbody_op_token, onk_defsig_op_token, 0},
  };

  const enum onk_lexicon_t expected = ONK_PARAM_OPEN_TOKEN;

  int8_t idx = is_dual_grp_keyword(current->type);
  assert(idx >= 0);

  ghead->expr_cnt += 1;
  push_many_ops(products[idx], current, state, 2);

  if(current->type == ONK_FOR_TOKEN)
  {
    // TODO: move to predict.c
    // Short
    if(next->type == onk_is_tok_open_brace(current->type)
       && next->type != ONK_PARAM_OPEN_TOKEN)
    {
      throw_unexpected_token(state, current, &expected, 1);
      return;
    }
    else
    {
      ghead = new_grp(state, (void *)op_push(ONK_BRACE_OPEN_TOKEN, 0, 0, state));
      state->set_ctr += 1;

      ghead->type = onk_tuple_group_token;
      ghead->is_short = true;
    }
  }
}

/*
 * pop short-block
 *
 * short blocks are isolated
 * & inferred groups
 * if(x) return y;
*/
void pop_short_block(struct onk_parser_state_t*state) {
  const struct onk_parse_group_t *ghead = 0;
  const struct onk_token_t *ophead = 0, *gmod = 0, *next = 0;
  /*
   TODO: check for ONK_ELSE_TOKEN before collapsing
   NOTE: WIP
  */
  next = next_token(state);
  if (next->type == ONK_ELSE_TOKEN)
    return;

  do {
    ghead = group_head(state);
    ophead = op_head(state);
    gmod = group_modifier(state, ghead);

    state->operators_ctr -= 1;
    assert(onk_is_tok_open_brace(ophead->type));

    if (onk_is_tok_group_modifier(gmod->type))
    {
      insert(state, ophead);
      state->operators_ctr -= 1;
    }

    state->set_ctr -= 1;

  } while(ghead->is_short && ghead->collapse);
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
int8_t handle_import(struct onk_parser_state_t*state)
{
  struct onk_parse_group_t *gtop = 0;
  const struct onk_token_t *current = current_token(state);

  gtop = new_grp(state, (void *)next_token(state));
  gtop->is_short = true;

  /* check tail of output */
  if(output_head(state)->type == ONK_FROM_TOKEN)
    /* pop from's short-block */
    pop_short_block(state);

  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  op_push(ONK_PARAM_OPEN_TOKEN, 0, 0, state);
  return 0;
}

/*
* flush the operator stack
* pop short blocks
*/
int8_t handle_delimiter(struct onk_parser_state_t *state)
{
  const enum onk_lexicon_t delim[2] = {ONK_COLON_TOKEN, ONK_SEMICOLON_TOKEN};

  struct onk_parse_group_t *ghead = group_head(state);
  const struct onk_token_t
    *current = current_token(state),
    *prev = prev_token(state);

  ghead->delimiter_cnt += 1;
  ghead->last_delim = current;

  flush_ops(state);

  // TODO: see how this plays with if/for/while
  if (!onk_is_tok_delimiter(prev->type))
    ghead->expr_cnt += 1;

  /* fill in empty index arg if non-specified*/
  if(ghead->type == onk_idx_group_token)
    idx_infer_value(state);

  if(ghead->is_short)
    pop_short_block(state);

  return 0;
}

// TODO: Move init code to top of file
int8_t handle_return(struct onk_parser_state_t*state)
{
  const struct onk_token_t *current = current_token(state);
  const struct onk_token_t *next = next_token(state);
  const struct onk_token_t *brace = 0;
  struct onk_parse_group_t *group = 0;

  /* push return onto operator stack */
  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  /*setup a group for its values*/
  if (!onk_is_tok_open_brace(next->type))
  {
    brace = op_push(ONK_BRACE_OPEN_TOKEN, 0, 0, state);
    group = new_grp(state, brace);
    group->is_short = true;
    group->collapse = true;
  }
  return 0;
}

int8_t onk_parse_step(struct onk_parser_state_t *state)
{
    const struct onk_token_t *current = 0;
    enum onk_lexicon_t current_type = ONK_EOF_TOKEN;
    bool unexpected_token = true;

    current = &state->tokens[*state->_i];
    current_type = state->tokens[*state->_i].type;

    assert(state->operator_stack_sz > state->operators_ctr);
    unexpected_token = is_token_unexpected(state);

    /*
      unexpected_token is a boolean used to determine if we
      failed on the previous loop, or the current one.

      if either state.panic or unexpected_token are true
      we begin unwinding
    */

    if(state->panic) {
      printf("paniced on last iter");
      //handle_unwind(&state, unexpected_token);
      return onk_parse_loopctl_panic;
    }
    else if (unexpected_token) {
      char * ptr = malloc(512);

      printf("got: [%s] expected: \n", onk_ptoken(current->type));

      onk_snprint_lex_arr(
        ptr,
        512,
        state->expect,

        //TODO FIXME
        (short)state->nexpect
      );

      printf("%s\n", ptr);
      free(ptr);

      return onk_parse_loopctl_panic;
      //handle_unwind(&state, unexpected_token);
    }

    /*
     * This should only be ran until
     * find_next is ran, afterwards,
    */
    else if (can_ignore_token(current_type))
      return onk_parse_loopctl_continue;

    else
      state->peek_next = find_next(state);

    /*
     * Units are the the foundational symbols
     * that represent proceedures & data
     * ONK_WORD_TOKENS, INTS, ONK_STRING_LITERAL_TOKEN
    */
    if(onk_is_tok_unit(current_type) || onk_is_tok_loopctlkw(current_type))
      insert(state, current);

    /*
     * Operations are pushed onto a stack,
     * and are popped off when an operator of greater
     * precedense is pushed onto the stack, or whenever
     * the operator-stack is flushed.
    */
    else if (onk_is_tok_operator(current_type))
      handle_operator(state);

    /*
      Places 2 operators on the stack, that are
      popped when an open brace is
      placed onto the stack */
    else if(is_dual_grp_keyword(current_type))
      handle_dual_group(state);

    /* flush operators and
     * pop an operator (open brace) & grouping */
    else if (onk_is_tok_close_brace(current_type))
      handle_close_brace(state);

    /* push group modifier & open brace
     * onto the operator stack.
     * push another grouping */
    else if (onk_is_tok_open_brace(current_type))
      handle_open_brace(state);

    /* flush operators, check for short grouping & pop it */
    else if (onk_is_tok_delimiter(current_type))
      handle_delimiter(state);

    else if(current_type == ONK_IMPORT_TOKEN)
      handle_import(state);

    /* pop from into output */
    else if(current_type == ONK_FROM_LOCATION)
    {
      insert(state, current);
      insert(state, op_head(state));
      state->operators_ctr -= 1;
    }

    else if(current_type == ONK_RETURN_TOKEN)
      handle_return(state);

    else if(current_type == ONK_EOF_TOKEN)
      return onk_parse_loopctl_finished;

    else {
#ifdef DEBUG
      printf("debug: token (%s) fell through precedence\n",
             onk_ptoken(current_type));
#endif
    }

    if(!state->panic)
      restoration_hook(state);

    state->peek_prev = *state->_i;

    if (state->peek_next != 0)
      *state->_i = state->peek_next;

    return onk_parse_loopctl_continue;
}


int8_t onk_parse_steps(struct onk_parser_state_t *state) {
    uint8_t finished = 0;

    for (*state->_i = 0; state->token_len > *state->_i; (*state->_i)++)
    {
      switch(onk_parse_step(state))
      {
          case onk_parse_loopctl_continue:
              break;

          case onk_parse_loopctl_panic:
              break;

          case onk_parse_loopctl_finished:
              finished = 1;
              break;

          /* TODO: if you want */
          /* case onk_parse_loopctl_unwind: */
          /*     handle_unwind() ? */
          /*     break;  */
      }

      if (finished)
          break;
  }
  return 0;
}

int8_t onk_rc_parse(
  struct onk_parser_input_t *input,
  struct onk_parser_output_t *out
){
  struct onk_parser_state_t state;
  uint16_t i = 0;

  onk_parser_construct(&state, &i, input);
  onk_parse_steps(&state);
  state._mode = onk_parser_exfil_mode;

  /* dump the remaining operators onto the output */
  flush_ops(&state);
  assert(state.operators_ctr == 1);

  onk_parser_output_from_parser_state(out, &state);
  onk_parser_free(&state);
  return 0;
}

struct onk_parser_output_t onk_parse(struct onk_parser_input_t *input)
{
    struct onk_parser_output_t output;

    onk_rc_parse(input, &output);

    return output;
}
