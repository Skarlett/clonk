/*
** predict.c
**
** This file is for determining the validity of the parser's input.
** It works by looking at the current token,
** and producing a collection of valid tokens that could occur
** next in the sequence.
**
**  Words & meaning:
**    predictor - A token that is apart of the collection of valid tokens
**
**
** During the parsing process, sometimes certain tokens must occur.
** This generally is in special conditions, these are referred to as
** "rules".
**
*/

/* TODO: ensure the following constrains on struct init */
/*      ONK_WORD_TOKEN { ONK_WORD_TOKEN=[expr], } */
/* TODO: allow for `{x;;}` but not `[x,,]` */
/* TODO: limit delimiters in index access 3 >= */
/* TODO: figure out if you can declare a new variable, if its valid */

/* `struct/def/impl` expects an explicit word/ident after its occurance */

#include <string.h>
#include "clonk.h"
#include "lexer.h"
#include "../private.h"
#include "predict.h"

bool is_expecting_data(enum onk_lexicon_t current)
{
  return onk_is_tok_operator(current)
     || onk_is_tok_delimiter(current)
     || current == ONK_IN_TOKEN;
}

enum onk_lexicon_t place_delimiter(struct Parser *state)
{
  struct Group *ghead = group_head(state);
  const struct onk_token_t *gmod = group_modifier(state, ghead);

  if (gmod->type == onk_while_cond_op_token
     || gmod->type == onk_ifcond_op_token)
     return ONK_UNDEFINED_TOKEN;

  switch (ghead->type) {
    case onk_idx_op_token:
      if(ghead->delimiter_cnt > 2)
        return ONK_UNDEFINED_TOKEN;
      return ONK_COLON_TOKEN;

    case onk_list_group_token:
      return ONK_COMMA_TOKEN;
    case onk_struct_group_token:
      return ONK_COMMA_TOKEN;
    case onk_tuple_group_token:
      return ONK_COMMA_TOKEN;

    case onk_map_group_token:
      if(ghead->delimiter_cnt % 2 == 0)
        return ONK_COLON_TOKEN;
      return ONK_COMMA_TOKEN;

    case onk_code_group_token:
      return ONK_SEMICOLON_TOKEN;

    default:
      return -1;
  }
}

enum onk_lexicon_t place_closing_brace(struct Parser *state)
{
  const struct onk_token_t *current = current_token(state);
  struct Group *ghead = group_head(state);
  const struct onk_token_t *gmod = group_modifier(state, ghead);

  if(current->type == ONK_PARAM_OPEN_TOKEN)
  {
    if(gmod->type == onk_ifcond_op_token
      || gmod->type == onk_while_cond_op_token)
      return ONK_UNDEFINED_TOKEN;
  }

  else if (current->type == ONK_BRACKET_OPEN_TOKEN
           && ghead->type == onk_idx_group_token)
           return ONK_UNDEFINED_TOKEN;

  return onk_invert_brace(ghead->origin->type);
}

bool _explicit_expecting_word(enum onk_lexicon_t current)
{
  return current == ONK_STRUCT_TOKEN
    || current == ONK_IMPL_TOKEN
    || current == ONK_DEF_TOKEN
    || current == ONK_IMPORT_TOKEN;
}

/* `if/while` expects an explicit onk_open_param_token after its occurance */
int8_t _explicit_expecting_open_param(enum onk_lexicon_t current)
{
  return current == ONK_IF_TOKEN
    || current == ONK_WHILE_TOKEN
    || current == ONK_FOR_TOKEN;
}

int8_t expect_operand(enum onk_lexicon_t current)
{
  return onk_is_tok_open_brace(current)
     || onk_is_tok_delimiter(current)
     || onk_is_tok_operator(current)
     || current == ONK_RETURN_TOKEN;
}

int8_t expect_operator(enum onk_lexicon_t current)
{
  return onk_is_tok_close_brace(current)
    || onk_is_tok_unit(current);
}

int8_t expect_default_expression(enum onk_lexicon_t current)
{
  return onk_is_tok_unit(current) || onk_is_tok_brace(current);
}

enum onk_lexicon_t handle_keyword(enum onk_lexicon_t current)
{
  if (_explicit_expecting_open_param(current))
    return ONK_PARAM_OPEN_TOKEN;

  else if(_explicit_expecting_word(current))
    return ONK_WORD_TOKEN;

  else if(current == ONK_ELSE_TOKEN)
   return ONK_BRACE_OPEN_TOKEN;

  else if(current == ONK_FROM_TOKEN)
    return ONK_FROM_LOCATION;

  return ONK_UNDEFINED_TOKEN;
}

int8_t default_expression(
  struct validator_t *validator,
  enum onk_lexicon_t current
){

  if(onk_is_tok_unit(current))
  {
    validator->slices[0] = (enum onk_lexicon_t *)&EXPR;
    validator->islices[0] = EXPR_LEN;
    validator->nslices = 1;

    switch(current)
    {
      case ONK_WORD_TOKEN:
        validator->buffer[0] = ONK_DOT_TOKEN;
        validator->buffer[1] = ONK_BRACKET_OPEN_TOKEN;
        validator->buffer[2] = ONK_PARAM_OPEN_TOKEN;
        validator->buffer[3] = ONK_BRACE_OPEN_TOKEN;
        validator->nbuffer = 4;

        validator->slices[0] = (enum onk_lexicon_t *)&ASNOP;
        validator->islices[0] = ASNOP_LEN;
        validator->nslices = 1;
        break;

      case ONK_STRING_LITERAL_TOKEN:
        validator->buffer[0] = ONK_DOT_TOKEN;
        validator->buffer[1] = ONK_BRACKET_OPEN_TOKEN;
        validator->nbuffer = 2;
        break;

      default:
        break;
    }
  }

  else if(onk_is_tok_open_brace(current)
    || onk_is_tok_delimiter(current)
    || onk_is_tok_operator(current))
  {
    validator->slices[0] = (enum onk_lexicon_t *)&EXPR;
    validator->islices[0] = EXPR_LEN;
    validator->nslices = 1;
  }

  else if(onk_is_tok_close_brace(current))
  {
    validator->slices[0] = (enum onk_lexicon_t *)&NEXT_CLOSE_BRACE;
    validator->islices[0] = NEXT_CLOSE_BRACE_LEN;
    validator->nslices = 1;
  }

  else if(current == ONK_DOT_TOKEN)
  {
    validator->buffer[0] = ONK_WORD_TOKEN;
    validator->nbuffer = 1;
  }

  else if (onk_is_tok_block_keyword(current))
  {
    validator->buffer[0] = handle_keyword(current);
    validator->nbuffer = 1;
  }

  else if(current == ONK_ELSE_TOKEN)
  {
    validator->buffer[0] = ONK_IF_TOKEN;
    validator->buffer[1] = ONK_BRACE_OPEN_TOKEN;
    validator->nbuffer = 2;
  }

  else return 0;
  return 1;
}

/* */
int8_t expect_for_arg(
  struct validator_t *validator,
  enum onk_lexicon_t current
){
  if (current == ONK_WORD_TOKEN)
  {
    validator->buffer[0] = ONK_COMMA_TOKEN;
    validator->nbuffer = 1;
  }
  else if(current == ONK_COMMA_TOKEN)
  {
    validator->buffer[0] = ONK_WORD_TOKEN;
    validator->nbuffer = 1;
  }
  else return -1;

  return 0;
}

bool use_parameter_mode(enum onk_lexicon_t gmod)
{
  return gmod == DefSign
    || gmod == ONK_STRUCT_TOKEN
    || gmod == onk_struct_init_op_token;
}

bool is_inside_for_args(
  enum onk_lexicon_t ophead,
  enum onk_lexicon_t gmod)
{
  return ophead == ONK_PARAM_OPEN_TOKEN
    && gmod == onk_for_args_op_token;
}

bool is_inside_impl_body(
  enum onk_lexicon_t ophead,
  enum onk_lexicon_t gmod,
  enum onk_lexicon_t current)
{
  return ophead == ONK_BRACE_OPEN_TOKEN && gmod == ONK_IMPL_TOKEN
          && (current == ONK_BRACE_OPEN_TOKEN
              || current == ONK_BRACE_CLOSE_TOKEN);
}

bool _start_block_after_param_grp(enum onk_lexicon_t ophead)
{
  return ophead == onk_defbody_op_token
      || ophead == onk_while_body_op_token
      || ophead == onk_ifbody_op_token;
}

/* use `{` next */
bool start_block(
  enum onk_lexicon_t current,
  enum onk_lexicon_t ophead
){
  return
    /*   def sig() { */
    /* while sig() { */
    /*  if(foobar) { */
    /*           ^^ */
    ((ophead == onk_defbody_op_token
      || ophead == onk_while_body_op_token
      || ophead == onk_ifbody_op_token)
      && current == ONK_PARAM_CLOSE_TOKEN)

    /* struct/impl word { */
    /*                ^-^ */
    || ((ophead == ONK_STRUCT_TOKEN
        || ophead == ONK_IMPL_TOKEN)
        && current == ONK_WORD_TOKEN)

    /* for x in expr { */
    /*             ^-^ */
    || (ophead == onk_for_body_op_token && onk_is_tok_unit(current));
}

int8_t ctx_paramter_mode(struct validator_t *validator, enum onk_lexicon_t current)
{

  switch(current) {
    case ONK_WORD_TOKEN:
      validator->buffer[0] = ONK_EQUAL_TOKEN;
      validator->nbuffer = 1;
      break;

    case ONK_COMMA_TOKEN:
      validator->buffer[0] = ONK_WORD_TOKEN;
      validator->nbuffer = 1;
      break;

    case ONK_EQUAL_TOKEN:
      validator->slices[0] = (enum onk_lexicon_t *)&EXPR;
      validator->islices[0] = EXPR_LEN;
      validator->nslices = 1;
      break;

    default: return -1;
  }

  return 0;
}

bool start_parameter_mode(enum onk_lexicon_t ophead, enum onk_lexicon_t current)
{
  return (ophead == DefSign && current == ONK_PARAM_OPEN_TOKEN)
    || ((ophead == ONK_STRUCT_TOKEN
         || ophead == onk_struct_init_op_token)
         && current == ONK_BRACE_OPEN_TOKEN);
}

/*
** special conditions inside of groups
**
*/
int8_t apply_group_rules(struct validator_t *validator, struct Parser *state)
{
  struct Group *ghead = group_head(state);
  enum onk_lexicon_t current = current_token(state)->type;
  enum onk_lexicon_t ophead = op_head(state)->type;
  enum onk_lexicon_t gmod = group_modifier(state, ghead)->type;

  /*
   * def sig(word=?expr, ..),
   * struct definition {word=?expr, ..},
   *
   * definition {word,  ...}
   * definition {word= expr, ..}
   *                 ^
  */
  if (use_parameter_mode(current)) {
    assert(ctx_paramter_mode(validator, current) == 0);
    return 1;
  }

  /*     def func(word) { }*/
  /*  struct foo {word}    */
  /* init_struct {word}    */
  /*              ^        */
  else if(start_parameter_mode(ophead, current))
  {
    validator->buffer[0] = ONK_WORD_TOKEN;
    validator->nbuffer = 1;
    return 1;
  }

  /*
  *         impl word {
  *       struct word {
  *         def foo() {
  *   for x in foobar {
  *    while (foobar) {
  *        if(foobar) {
  *                   ^
  * This isn't an exhaustive list,
  * but those based on parsing context
  */
  else if(start_block(ophead, current))
  {
    validator->buffer[0] = ONK_BRACE_OPEN_TOKEN;
    validator->nbuffer = 1;
    return 1;
  }

  /* special handling for certain groups & modifiers */
  else if(is_inside_for_args(ophead, gmod))
  {
    expect_for_arg(validator, current);
    return 1;
  }

  /* impl word { def foo() {...} }*/
  /*             ^                */
  else if(is_inside_impl_body(ophead, gmod, current))
  {
    validator->buffer[0] = ONK_BRACE_OPEN_TOKEN;
    validator->nbuffer = 1;
    return 1;
  }

  return 0;
}

int8_t next_frame(
  struct validator_t *validator,
  struct Parser *state
){
  enum onk_lexicon_t current = current_token(state)->type;
  enum onk_lexicon_t ophead = op_head(state)->type;
  enum onk_lexicon_t delim;

  if(apply_group_rules(validator, state) == 0)
    default_expression(validator, current);

  delim = place_delimiter(state);

  if(delim != ONK_UNDEFINED_TOKEN)
  {
    validator->buffer[validator->nbuffer] = delim;
    validator->nbuffer += 1;
  }

  /* add keywords */
  if (ophead == ONK_BRACKET_OPEN_TOKEN)
  {
    validator->slices[validator->nslices] = (enum onk_lexicon_t *)KWORD_BLOCK;
    validator->islices[validator->nslices] = KWORD_BLOCK_LEN;
    validator->nslices += 1;
  }

  return 0;
}

uint16_t fill_buffer(
  enum onk_lexicon_t *arr,
  uint16_t arr_sz,
  struct validator_t *validator
){
  uint16_t total = 0;
  uint8_t islices = 0;
  uint16_t i;

  assert(validator->nbuffer > arr_sz);

  assert(memcpy(arr, validator->buffer,
    sizeof(enum onk_lexicon_t) * validator->nbuffer) != 0);

  total += validator->nbuffer;

  for (i=0; validator->nslices > i; i++)
  {
    islices = validator->islices[i];

    if(total + islices > arr_sz)
      return -1;

    total += islices;

    assert(memcpy(arr, validator->slices[i],
      sizeof(enum onk_lexicon_t) * islices) != 0);
  }

  return total;
}

/*
 * Write `_EX_EXPR` into state->expecting
*/
void init_expect_buffer(enum onk_lexicon_t *arr)
{
  assert(memcpy(arr, &KWORD_BLOCK, KWORD_BLOCK_SZ) != 0);
  assert(memcpy(arr, &EXPR, EXPR_SZ) != 0);
}

bool is_token_unexpected(struct Parser *state)
{
  enum onk_lexicon_t current = current_token(state)->type;
  struct validator_t frame;

  enum onk_lexicon_t *expecting = state->expect;
  uint16_t ex_capacity = state->expect_capacity;
  uint16_t ex_len = state->nexpect;

  bool found = false;

  assert(ex_len > 0);

  for(uint16_t i = 0; ex_len > i; i++)
  {
    if(expecting[i] == current)
    {
      found = true;
      break;
    }
  }

  if (found == false)
    return 1;

  next_frame(&frame, state);

  state->nexpect = fill_buffer(expecting, ex_capacity, &frame);

  return 0;
}
