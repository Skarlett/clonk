
#include <string.h>
#include <assert.h>
#include "clonk.h"
#include "private.h"

/*
  Add 2 unsigned 16bit integers within bounds
*/
uint16_t onkstd_add_u16(uint16_t a, uint16_t b)
{
  if (UINT16_MAX == a || UINT16_MAX == b)
    return UINT16_MAX;

  else if(UINT16_MAX - a > b)
    return a + b;

  return UINT16_MAX;
}

uint16_t onkstd_sub_u16(uint16_t a, uint16_t b)
{
  if (b > a) return 0;

  return a - b;
}

void assert_op_pop_n(const struct Parser *state, uint16_t n)
{
  assert(state->operators_ctr >= n);
}

void assert_op_push_n(const struct Parser *state, uint16_t n)
{
  assert(UINT16_MAX - state->operators_ctr >= n);
}

void assert_vec_len(const struct onk_vec_t *vec, uint16_t n)
{
  assert(vec->len >= n);
}

/* push to output */
void insert(struct Parser *state, const struct onk_token_t *tok)
{
  assert(onk_vec_push(&state->debug, &tok) != 0);
}


/* push token into pool */
const struct onk_token_t * new_token(struct Parser *state, struct onk_token_t *tok) {
  return onk_vec_push(&state->pool, tok);
}

void insert_new(
  enum onk_lexicon_t type,
  uint16_t start,
  uint16_t end,
  struct Parser *state
){

  struct onk_token_t tok;
  const struct onk_token_t *heap;

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
uint16_t find_next(struct Parser *state) {

  for (uint16_t i = *state->_i; state->src_sz > i; i++)
  {
    if(can_ignore_token(state->src[i].type))
      continue;

    return i;
  }

  return 0;
}


const struct onk_token_t * prev_token(const struct Parser *state) {
  if (*state->_i == 0)
    return 0;
  return &state->src[state->peek_prev];
}

const struct onk_token_t * next_token(const struct Parser *state) {
  if(state->peek_next == 0)
    return 0;
  return &state->src[state->peek_next];
}

/*
**
 */
const struct onk_token_t * current_token(const struct Parser *state){
  return &state->src[*state->_i];
}


struct Group * group_head(struct Parser *state){
  if (state->set_sz > state->set_ctr && state->set_ctr > 0)
    return &state->set_stack[state->set_ctr - 1];
  return 0;
}

const struct onk_token_t * op_head(const struct Parser *state){
  if(state->operators_ctr > 0)
    return state->operator_stack[state->operators_ctr - 1];
  return 0;
}

const struct onk_token_t * output_head(const struct Parser *state) {
  return onk_vec_head(&state->debug);
}

const struct onk_token_t * group_modifier(
  const struct Parser *state,
  const struct Group *group)
{
  const struct onk_token_t *modifier;

  if (group->operator_idx > 0)
  {
    modifier = state->operator_stack[group->operator_idx - 1];

    if (onk_is_tok_group_modifier(modifier->type))
      return modifier;
  }

  return 0;
}


enum onk_lexicon_t group_type_init(enum onk_lexicon_t brace)
{

  switch (brace) {
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

/*
** pushes group into output as a group token
*/
int8_t push_group(struct Parser *state, const struct Group *grp) {

  //if(!onk_is_tok_open_brace(brace->type))
  //  return -1;

  //TODO: implement
}

struct Group * new_grp(
  struct Parser * state,
  const struct onk_token_t * from
){
  struct Group *ghead;
  assert(state->set_ctr < state->set_sz);


  ghead = &state->set_stack[state->set_ctr];
  state->set_ctr += 1;

  /* assumes `OPEN_BRACE/PARAM/BRACKET` is on the operator stack*/
  ghead->operator_idx = state->operators_ctr - 1;
  ghead->set_idx = state->set_ctr - 1;

  ghead->last_delim = 0;
  ghead->delimiter_cnt = 0;
  ghead->expr_cnt = 0;

  ghead->is_short = false;
  ghead->collapse = false;

  ghead->origin = from;
  ghead->type = group_type_init(from->type);

  return ghead;
}

bool is_index_pattern(const struct onk_token_t *prev) {
  return onk_is_tok_close_brace(prev->type)
    || prev->type == ONK_WORD_TOKEN
    || prev->type == ONK_STRING_LITERAL_TOKEN;
}

bool is_fncall_pattern(const struct onk_token_t *prev){
  return onk_is_tok_close_brace(prev->type)
    || prev->type == ONK_WORD_TOKEN;
}

const struct onk_token_t * op_push(enum onk_lexicon_t op, uint16_t start, uint16_t end, struct Parser *state)
{
  struct onk_token_t new, *heap = 0;
  assert_op_push_n(state, 1);


  new.type = op;
  new.end = end;
  new.start = start;

  heap = onk_vec_push(&state->pool, &new);
  assert(heap);
  
  state->operator_stack[state->operators_ctr] = heap;  
  state->operators_ctr += 1;
  return heap;
}

int8_t push_many_ops(
  const enum onk_lexicon_t *ops,
  const struct onk_token_t *origin,
  struct Parser *state,
  uint16_t nitems
){
  struct onk_token_t tmp;
  const struct onk_token_t *heap;

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
  struct Parser *state,
  enum onk_lexicon_t type,
  uint16_t argc
){
  struct onk_token_t marker;
  const struct onk_token_t *ret;
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
int8_t flush_ops(struct Parser *state)
{
  const struct onk_token_t *head;

  while (state->operators_ctr > 0) {
    head = op_head(state);

    /* ends if tokens inverted brace is found*/
    if (op_precedence(head->type) == 0)
      break;

    /* otherwise pop into output */
    else {
      insert(state, head);
      state->operators_ctr -= 1;
    }
  }

  return 0;
}

int8_t finish_idx_access(struct Parser *state)
{
  struct Group *ghead = group_head(state);
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
      "^"       : 8 R (1 ^ 2 ^ 3) -> (1 ^ (2 ^ 3))
      "/ * %"   : 7 L  (4 / 2 * 2) -> ((4 / 2) * 2)
      "+ -"     : 6 L
      "! ~"     : 5 R
      ">> << | &": 4
      "!= == >= > <= < && || in": 3 L
      "+= -= = &= |= ~=": 1 R
      "( [ { ONK_IF_TOKEN ONK_ELSE_TOKEN ONK_RETURN_TOKEN DEF" : 0 non-assoc
*/
#define END_PRECEDENCE 127
int8_t op_precedence(enum onk_lexicon_t token) {
    
    if (onk_is_tok_close_brace(token))
        return END_PRECEDENCE;
    
    if (token == ONK_DOT_TOKEN)
        return 126;
    
    else if (token == ONK_POW_TOKEN)
        return 6;

    else if (token == ONK_MUL_TOKEN 
        || token == ONK_DIV_TOKEN
        || token == ONK_MOD_TOKEN)
        return 5;

    else if (token == ONK_ADD_TOKEN
        || token == ONK_SUB_TOKEN)
        return 4;
    
    else if (token == ONK_ISEQL_TOKEN
        || token == ONK_NOT_EQL_TOKEN
        || token == ONK_GT_EQL_TOKEN
        || token == ONK_LT_EQL_TOKEN
        || token == ONK_GT_TOKEN
        || token == ONK_LT_TOKEN
        || token == ONK_AND_TOKEN
        || token == ONK_OR_TOKEN
        || token == ONK_NOT_TOKEN)
        return 2;
    
    else if (token == ONK_EQUAL_TOKEN
      || token == ONK_PLUSEQ_TOKEN
      || token == ONK_MINUS_EQL_TOKEN)
      return 1;
    
    else if (onk_is_tok_open_brace(token)
      || onk_is_tok_group_modifier(token))
      return 0;
    
    return -1;
}

int8_t init_parser(
  struct Parser *state,
  const struct ParserInput *in,
  uint16_t *i
){
  if (
    onk_vec_init(&state->pool, 256, sizeof(struct onk_token_t)) == -1
    ||onk_vec_init(&state->debug, 2048, sizeof(void *)) == -1
    ||onk_vec_init(&state->errors, 64, sizeof(struct ParserError)) == -1
    ||onk_vec_init(&state->restoration_stack, 2048, sizeof(struct RestorationFrame)) == -1
  ) return -1;

  state->src_code = in->src_code;
  state->_i = i;

  state->src = in->tokens.base;
  state->src_sz = in->tokens.len;

  state->set_ctr = 0;
  state->set_sz = ONK_STACK_SZ;

  state->operators_ctr = 0;
  state->operator_stack_sz = ONK_STACK_SZ;

  state->peek_next = 0;
  state->peek_prev = 0;

  init_expect_buffer(&state->expecting);

  assert(op_push(ONK_BRACE_OPEN_TOKEN, 0, 0, state) != 0);
  return 0;
}

int8_t parser_free(struct Parser *state) {
  if (
    onk_vec_free(&state->debug) == -1
    || onk_vec_free(&state->pool) == -1
    || onk_vec_free(&state->errors) == -1
    || onk_vec_free(&state->restoration_stack) == -1)
    return -1;

  return 0;
}

int8_t parser_reset(struct Parser *state)
{
  memset(state->operator_stack, 0, sizeof(void *[ONK_STACK_SZ]));
  state->operators_ctr = 0;

  memset(state->set_stack, 0, sizeof(void *[ONK_STACK_SZ]));
  state->set_ctr = 0;

  state->src_sz = 0;
  state->src = 0;
  state->_i = 0;
  state->src_code = 0;

  init_expect_buffer(&state->expecting);

  parser_free(state);
  return 0;
}

void parser_input_from_lexer_output(
  const struct onk_lexer_output_t *lex,
  struct ParserInput *parser_in,
  bool add_glob_scope)
{
  parser_in->src_code = lex->src_code;
  parser_in->src_code_sz = lex->src_code_sz;
  memcpy(&parser_in->tokens, &lex->tokens, sizeof(struct onk_vec_t));
  parser_in->add_glob_scope = add_glob_scope;
}
