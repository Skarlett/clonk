#include <string.h>
#include "clonk.h"
#include "parser.h"
#include "private.h"
#include "semantics.h"

void assert_op_pop_n(const struct onk_parser_state_t*state, uint16_t n)
{
  assert(state->operators_ctr >= n);
}

void assert_op_push_n(const struct onk_parser_state_t*state, uint16_t n)
{
  assert(UINT16_MAX - state->operators_ctr >= n);
}

void assert_vec_len(const struct onk_vec_t *vec, uint16_t n)
{
  assert(vec->len >= n);
}

/* push to output */
void insert(struct onk_parser_state_t*state, const struct onk_token_t *tok)
{
  assert(onk_vec_push(&state->debug, &tok) != 0);
}

/* push token into pool */
const struct onk_token_t * new_token(struct onk_parser_state_t*state, struct onk_token_t *tok) {
  return onk_vec_push(&state->pool, tok);
}

void insert_new(
  enum onk_lexicon_t type,
  uint16_t start,
  uint16_t end,
  struct onk_parser_state_t*state
){

  struct onk_token_t tok;
  const struct onk_token_t *heap = 0;

  tok.start = start;
  tok.end = end;
  tok.type = type;
  tok.seq = 0;

  heap = new_token(state, &tok);
  insert(state, heap);
}

/*
**
** TODO: Now that the parser accepts white-space
** We have to ensure that `next_token`,
** `prev_token`
**
**
*/
bool can_ignore_token(enum onk_lexicon_t tok)
{
  return onk_is_tok_whitespace(tok) || tok == ONK_COMMENT_TOKEN;
}

/*
 * assuming the current token is not ignored-type
 * give the position of the next non-ignored typed token
 */
uint16_t find_next(struct onk_parser_state_t*state) {

  for (uint16_t i = *state->_i; state->token_len > i; i++)
  {
    if(can_ignore_token(state->tokens[i].type))
      continue;

    return i;
  }

  return 0;
}

const struct onk_token_t * prev_token(const struct onk_parser_state_t*state) {
  if (*state->_i == 0)
    return 0;
  return &state->tokens[state->peek_prev];
}

const struct onk_token_t * next_token(const struct onk_parser_state_t*state) {
  if(state->peek_next == 0)
    return 0;
  return &state->tokens[state->peek_next];
}

const struct onk_token_t * current_token(struct onk_parser_state_t*state){
  return &state->tokens[*state->_i];
}

struct onk_parse_group_t * group_head(struct onk_parser_state_t*state){
  if (state->set_sz > state->set_ctr && state->set_ctr > 0)
    return &state->set_stack[state->set_ctr - 1];
  return 0;
}

const struct onk_token_t * op_head(const struct onk_parser_state_t*state){
  if(state->operators_ctr > 0)
    return state->operator_stack[state->operators_ctr - 1];
  return 0;
}

const struct onk_token_t * output_head(const struct onk_parser_state_t*state) {
  return onk_vec_head(&state->debug);
}

const struct onk_token_t * group_modifier(
  const struct onk_parser_state_t*state,
  const struct onk_parse_group_t *group)
{
  const struct onk_token_t *modifier = 0;

  if (group->operator_idx > 0)
  {
    modifier = state->operator_stack[group->operator_idx - 1];

    if (onk_is_tok_group_modifier(modifier->type))
      return modifier;
  }

  return 0;
}


void onk_parser_free(struct onk_parser_state_t *state)
{
    assert(state->_mode >= onk_parser_free_mode);
    onk_vec_free(&state->restoration_stack);
    state->_mode = onk_parser_uninit_mode;
}

int8_t onk_parser_reset(struct onk_parser_state_t *state)
{
    onk_vec_clear(&state->debug);
    onk_vec_clear(&state->pool);
    onk_vec_clear(&state->errors);
    onk_vec_clear(&state->restoration_stack);

    state->operators_ctr = 0;
    state->set_ctr = 0;

    state->peek_next = 0;
    state->peek_prev = 0;
    state->_mode = onk_parser_alloc_mode;

    // tokens
    state->token_len = 0;
    state->tokens = 0;

    // raw input
    state->_i = 0;
    return 0;
}

bool is_index_pattern(enum onk_lexicon_t prev) {
  return onk_is_tok_close_brace(prev)
    || prev == ONK_WORD_TOKEN
    || prev == ONK_STRING_LITERAL_TOKEN;
}

bool is_fncall_pattern(enum onk_lexicon_t prev){
  return onk_is_tok_close_brace(prev)
    || prev == ONK_WORD_TOKEN;
}

void idx_infer_value(struct onk_parser_state_t*state)
{
  const struct onk_token_t *prev = prev_token(state);
  if(prev) {
    if (prev->type == ONK_COLON_TOKEN || prev->type == ONK_BRACKET_OPEN_TOKEN)
      push_output(state, ONK_NULL_TOKEN, 0);
  }
}

const struct onk_token_t * op_push(enum onk_lexicon_t op, uint16_t start, uint16_t end, struct onk_parser_state_t*state)
{
  struct onk_token_t new, *heap = 0;
  assert_op_push_n(state, 1);

  new.type = op;
  new.end = end;
  new.start = start;

  /* place token_t struct in heap */
  heap = onk_vec_push(&state->pool, &new);
  assert(heap);
  
  state->operator_stack[state->operators_ctr] = heap;  
  state->operators_ctr += 1;
  return heap;
}

int8_t push_many_ops(
  const enum onk_lexicon_t *ops,
  const struct onk_token_t *origin,
  struct onk_parser_state_t*state,
  uint16_t nitems
){
  struct onk_token_t tmp;
  const struct onk_token_t *heap = 0;

  assert_op_push_n(state, nitems);

  tmp.start = origin->start;
  tmp.end = origin->end;

  for (uint16_t i = 0; nitems > i; i++)
  {
    tmp.type = ops[i];
    heap = new_token(state, &tmp);
    assert(heap);

    state->operator_stack[state->operators_ctr] = heap;
    state->operators_ctr += 1;
  }

  return 0;
}

void push_output(
  struct onk_parser_state_t*state,
  enum onk_lexicon_t type,
  uint16_t argc
){
  struct onk_token_t marker;
  const struct onk_token_t *ret = 0;
  assert(type != ONK_UNDEFINED_TOKEN);

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
** Flushes items from stack until a precedense of
** 0 is found, or stack empty.
**
** Items with 0 precedense value:
**   Opening-braces
*/
int8_t flush_ops(struct onk_parser_state_t*state)
{
  const struct onk_token_t *head = 0;
  int8_t precedence = 0;

  while (state->operators_ctr > 0)
  {
    head = op_head(state);
    precedence = op_precedence(head->type);

    /* ends if tokens inverted brace is found*/
    if (precedence == -1)
      return -1;

    else if (precedence == 0)
      break;

    /* otherwise pop into output */
    else {
      insert(state, head);
      state->operators_ctr -= 1;
    }
  }

  return 0;
}

int8_t finish_idx_access(struct onk_parser_state_t*state)
{
  struct onk_parse_group_t *ghead = group_head(state);
  const struct onk_token_t *prev = prev_token(state);
  assert(ghead->type == onk_idx_group_token);

  /*
    peek-behind if token was ONK_COLON_TOKEN
    add a value for its missing argument
  */
  // a: -> a:a
  // :: => ::a
  if (prev->type == ONK_COLON_TOKEN)
    push_output(state, ONK_NULL_TOKEN, 0);

  // a:a -> a:a:a
  for (uint16_t i=ghead->expr_cnt; 3 > i; i++)
    push_output(state, ONK_NULL_TOKEN, 0);

  push_output(state, onk_idx_op_token, 0);

  return 0;
}

/*
    precendense table:
      ") ] }"   : 127 non-assoc
      "."       : 126 L
      "! ~":    : 13 R
      "^"       : 12 R (1 ^ 2 ^ 3) -> (1 ^ (2 ^ 3))
      "/ * %"   : 11 L  (4 / 2 * 2) -> ((4 / 2) * 2)
      "+ -"     : 10 L
      ">> <<"   : 9 L
     "> < >= <=": 8 L
     "==" "!="  : 7 L
      "&":      : 6 L
      "|"       : 5 L
      "&&"      : 4 L
      "||"      : 3 L
      "= |= &= ~= += -=" : 2 L
      "( [ { keywords "  : 0 non-assoc
*/
#define END_PRECEDENCE 127
int8_t op_precedence(enum onk_lexicon_t token) {
    
    if (onk_is_tok_close_brace(token))
        return END_PRECEDENCE;
    
    if (token == ONK_DOT_TOKEN)
        return 126;

    else if(onk_is_tok_unary_operator(token))
      return 13;

    else if (token == ONK_POW_TOKEN)
        return 12;

    else if (
        token == ONK_MUL_TOKEN
        || token == ONK_DIV_TOKEN
        || token == ONK_MOD_TOKEN)
        return 11;

    else if (
        token == ONK_ADD_TOKEN
        || token == ONK_SUB_TOKEN)
        return 10;

    else if(
      token == ONK_SHL_TOKEN
      || token == ONK_SHR_TOKEN)
      return 9;

    else if(
      token == ONK_GT_TOKEN
      || token == ONK_LT_TOKEN
      || token == ONK_LT_EQL_TOKEN
      || token == ONK_GT_EQL_TOKEN)
      return 8;

    else if(
      token == ONK_ISEQL_TOKEN
      || token == ONK_NOT_EQL_TOKEN)
      return 7;

    else if(token == ONK_AMPER_TOKEN)
      return 6;

    else if(token == ONK_PIPE_TOKEN)
      return 5;

    else if(token == ONK_AND_TOKEN)
      return 4;

    else if(token == ONK_OR_TOKEN)
      return 3;

    else if (
      token == ONK_EQUAL_TOKEN
      || token == ONK_BIT_OR_EQL
      || token == ONK_BIT_AND_EQL
      || token == ONK_BIT_NOT_EQL
      || token == ONK_PLUSEQ_TOKEN
      || token == ONK_MINUS_EQL_TOKEN)
      return 2;

    else if (onk_is_tok_open_brace(token)
      || onk_is_tok_group_modifier(token))
      return 0;
    
    return -1;
}

void onk_parser_input_from_lexer_output(
  const struct onk_lexer_output_t *lex,
  struct onk_parser_input_t *parser_in,
  bool add_glob_scope)
{
  parser_in->add_glob_scope = add_glob_scope;

  //TODO: FIXME
  //&lex->tokens is a vector,
  //but parser_tokens is a bare ``struct token_t[]``
  //onk_vec_move(&parser_in->tokens, &lex->tokens);
  parser_in->tokens = lex->tokens;
}

void onk_parser_output_from_parser_state(
    struct onk_parser_output_t *out,
    struct onk_parser_state_t *state
){
  assert(state->_mode == onk_parser_exfil_mode);
  onk_vec_move(&out->postfix, &state->debug);
  onk_vec_move(&out->token_pool, &state->pool);
  onk_vec_move(&out->errors, &state->errors);
  out->stage_failed = state->stage_failed;
  state->_mode = onk_parser_free_mode;
}
