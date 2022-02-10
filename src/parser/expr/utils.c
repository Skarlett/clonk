
#include <string.h>
#include <assert.h>
#include "expr.h"
#include "utils.h"
#include "../../utils/vec.h"

/* push to output */
void insert(struct Parser *state, const struct Token *tok) {
  assert(vec_push(&state->debug, &tok) != 0);
}

/* push token into pool */
const struct Token * new_token(struct Parser *state, struct Token *tok) {
  return vec_push(&state->pool, tok);
}

const struct Token * prev_token(const struct Parser *state) {
  if (*state->_i != 0)
    return &state->src[*state->_i - 1];
  return 0;
}

const struct Token * next_token(const struct Parser *state) {
  if(UINT16_MAX > *state->_i && state->src_sz > *state->_i)
    return &state->src[*state->_i + 1];
  return 0;
}

/*
** NOTE: Assumes `_i` is within bounds.
*/
const struct Token * current_token(const struct Parser *state){
  return &state->src[*state->_i];
}

struct Group * group_head(struct Parser *state){
  if (STACK_SZ - 1 > state->set_ctr && state->set_ctr > 0)
    return &state->set_stack[state->set_ctr - 1];
  return 0;
}

const struct Token * op_head(const struct Parser *state){
  if(state->operators_ctr > 0)
    return state->operator_stack[state->operators_ctr - 1];
  return 0;
}

const struct Token * output_head(const struct Parser *state) {
  return vec_head(&state->debug);
}

const struct Token * group_modifier(
  const struct Parser *state,
  const struct Group *group
){
  const struct Token *modifier;

  if (group->operator_idx > 0)
  {
    modifier=state->operator_stack[group->operator_idx - 1];

    if (is_group_modifier(modifier->type))
      return modifier;
  }

  return 0;
}


const group_type(
  struct Parser *state,
  const struct Token *brace,
  const struct Group *group
){


  group->operator_idx
}

/*
** pushes group into output as a group token
** NOTE: Assumes token->origin is a brace type;
*/
int8_t push_group(struct Parser *state, const struct Group *grp) {

  //if(!is_open_brace(brace->type))
  //  return -1;

  //TODO: implement
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

struct Group * new_grp(
  struct Parser * state,
  const struct Token * from
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
  ghead->short_type = sh_udef_t;

  ghead->origin = from;

  ghead->type = /*TODO*/ 0;

  return ghead;
}

bool is_index_pattern(const struct Token *prev) {
  return is_close_brace(prev->type)
    || prev->type == WORD
    || prev->type == STRING_LITERAL;
}

bool is_fncall_pattern(const struct Token *prev){
  return is_close_brace(prev->type)
    || prev->type == WORD;
}

const struct Token * op_push(enum Lexicon op, uint16_t start, uint16_t end, struct Parser *state)
{
  struct Token new, *heap = 0;
  new.type = op;
  new.end = end;
  new.start = start;

  heap = vec_push(&state->pool, &new);
  assert(heap);
  
  state->operator_stack[state->operators_ctr] = heap;  
  state->operators_ctr += 1;
  return heap;
}

int8_t push_many_ops(
  const enum Lexicon *ops,
  const struct Token *origin,
  struct Parser *state
){
  struct Token tmp;
  const struct Token *heap;
  tmp.start = origin->start;
  tmp.end = origin->end;

  for (uint16_t i = 0 ;; i++)
  {

    if(ops[i] == 0)
      break;

    //assert(state->operators_ctr < state->operator_stack_sz);

    tmp.type = ops[i];

    heap = new_token(state, &tmp);
    assert(heap);

    state->operator_stack[state->operators_ctr] = heap;
    state->operators_ctr += 1;
  }

  return 0;
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
  const struct Token *head = 0;
  head = state->operator_stack[(state->operators_ctr || 1) - 1];

  if (state->operators_ctr == 0)
    return 0;

  while (state->operators_ctr > 0) {
    /* ends if tokens inverted brace is found*/
    if (op_precedence(head->type) == 0)
      break;

    /* otherwise pop into output */
    else {
      insert(state, head);
      state->operators_ctr -= 1;
    }

    /* Grab the head of the stack */
    head = state->operator_stack[state->operators_ctr - 1];
  }

  return 0;
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

bool is_dual_grp_keyword(enum Lexicon tok) {
  return tok == FOR
    || tok == WHILE
    || tok == IF
    || tok == FUNC_DEF;
}

int8_t init_parser(
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

  init_expect_buffer(&state->expecting);

  assert(op_push(BRACE_OPEN, 0, 0, state) != 0);
  return 0;
}

int8_t parser_free(struct Parser *state) {
  if (
    vec_free(&state->debug) == -1
    || vec_free(&state->pool) == -1
    || vec_free(&state->errors) == -1
    || vec_free(&state->restoration_stack) == -1)
    return -1;

  return 0;
}

int8_t parser_reset(struct Parser *state)
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

  parser_free(state);
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
