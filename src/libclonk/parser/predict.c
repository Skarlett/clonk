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
**
**
** TODO: We need a datastructure which can handle validation because 
   this thing is hard to transverse entirely as to what
   should be allowed in the next validation frame.
   There seems to be no documentation that I left 
   behind which described 
   the transformation rules of the predictor.

   In otherwords, this thing should probably be rewritten.
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
#include "private.h"
#include "semantics.h"

/**
Returns COLON|SEMICOLON|COMMA|None (0)
**/
const enum onk_lexicon_t EXP_EXPLICIT_COLON = ONK_COLON_TOKEN;
const enum onk_lexicon_t EXP_EXPLICIT_SEMI_COLON = ONK_SEMICOLON_TOKEN;
const enum onk_lexicon_t EXP_EXPLICIT_COMMA = ONK_COMMA_TOKEN;
const enum onk_lexicon_t EXP_EXPLICIT_OPEN_PARAM = ONK_PARAM_OPEN_TOKEN;
const enum onk_lexicon_t EXP_EXPLICIT_WORD = ONK_WORD_TOKEN;
const enum onk_lexicon_t EXP_EXPLICIT_OPEN_BRACE = ONK_BRACE_OPEN_TOKEN;
const enum onk_lexicon_t EXP_EXPLICIT_FROM_LOC = ONK_FROM_LOCATION;
const enum onk_lexicon_t EXP_EXPLICIT_EQL = ONK_EQUAL_TOKEN;
const enum onk_lexicon_t EXP_EXPLICIT_WHITESPACE = ONK_WHITESPACE_TOKEN;

const enum onk_lexicon_t UNIT[UNIT_LEN] = {_EX_UNIT};
const enum onk_lexicon_t BINOP[BINOP_LEN] = {_EX_BIN_OPERATOR};
const enum onk_lexicon_t ASNOP[ASNOP_LEN] = {_EX_ASN_OPERATOR};
const enum onk_lexicon_t UNIOP[UNIOP_LEN] = {_EX_UNARY_OPERATOR};
const enum onk_lexicon_t OPEN_BRACE[BRACE_OPEN_LEN] = {_EX_OPEN_BRACE};
const enum onk_lexicon_t CLOSE_BRACE[BRACE_CLOSE_LEN] = {_EX_CLOSE_BRACE};
const enum onk_lexicon_t EXPR[EXPR_LEN] = {_EX_EXPR};
const enum onk_lexicon_t KWORD_BLOCK[KWORD_BLOCK_LEN] = {_EX_KWORD_BLOCK};
const enum onk_lexicon_t NEXT_CLOSE_BRACE[] = { _NEXT_CLOSE_BRACE};

bool is_expecting_data(enum onk_lexicon_t current)
{
  return onk_is_tok_operator(current)
     || onk_is_tok_delimiter(current)
     || current == ONK_IN_TOKEN;
}

const enum onk_lexicon_t * place_delimiter(struct onk_parser_state_t*state)
{
  struct onk_parse_group_t *ghead = group_head(state);
  const struct onk_token_t *gmod = group_modifier(state, ghead);

  if (gmod == 0 
     ||gmod->type == onk_while_cond_op_token
     || gmod->type == onk_ifcond_op_token)
     return 0;

  switch (ghead->type) {
    case onk_idx_op_token:
      if(ghead->delimiter_cnt > 2)
        return 0;
      return &EXP_EXPLICIT_COMMA;

    case onk_list_group_token:
      return &EXP_EXPLICIT_COMMA;
    
    case onk_struct_group_token:
      return &EXP_EXPLICIT_COMMA;
    
    case onk_tuple_group_token:
      return &EXP_EXPLICIT_COMMA;

    case onk_map_group_token:
      if(ghead->delimiter_cnt % 2 == 0)
        return &EXP_EXPLICIT_COLON;
      return &EXP_EXPLICIT_COMMA;

    case onk_code_group_token:
      return &EXP_EXPLICIT_SEMI_COLON;

    default:
      return 0;
  }
}

enum onk_lexicon_t place_closing_brace(struct onk_parser_state_t*state)
{
  const struct onk_token_t *current = current_token(state);
  struct onk_parse_group_t *ghead = group_head(state);
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

int8_t is_expect_operator(enum onk_lexicon_t current)
{
  return onk_is_tok_close_brace(current)
    || onk_is_tok_unit(current);
}

int8_t expect_operand(enum onk_lexicon_t current)
{
  return onk_is_tok_open_brace(current)
     || onk_is_tok_delimiter(current)
     || onk_is_tok_operator(current)
     || current == ONK_RETURN_TOKEN;
}

int8_t is_expect_operand(enum onk_lexicon_t current)
{
  return onk_is_tok_open_brace(current)
     || onk_is_tok_delimiter(current)
     || onk_is_tok_operator(current)
     || current == ONK_RETURN_TOKEN;
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

int8_t expect_default_expression(enum onk_lexicon_t current)
{
  return onk_is_tok_unit(current) || onk_is_tok_brace(current);
}

const enum onk_lexicon_t * handle_keyword(enum onk_lexicon_t current)
{
  if (_explicit_expecting_open_param(current))
    return &EXP_EXPLICIT_OPEN_PARAM;

  else if(_explicit_expecting_word(current))
    return &EXP_EXPLICIT_WORD;

  else if(current == ONK_ELSE_TOKEN)
    return &EXP_EXPLICIT_OPEN_BRACE;

  else if(current == ONK_FROM_TOKEN)
    return &EXP_EXPLICIT_FROM_LOC;

  return 0;
}

const enum onk_lexicon_t EXP_WORD[] = {
  ONK_DOT_TOKEN, ONK_BRACKET_OPEN_TOKEN,
  ONK_PARAM_OPEN_TOKEN, ONK_BRACE_OPEN_TOKEN
};
const uint16_t EXP_WORD_LEN = 4;

const enum onk_lexicon_t EXP_STRING[] = {
  ONK_DOT_TOKEN, ONK_BRACKET_OPEN_TOKEN
};
const uint16_t EXP_STRING_LEN = 2;

const enum onk_lexicon_t EXP_ELSE[] = {
  ONK_IF_TOKEN, ONK_BRACE_OPEN_TOKEN
};
const uint16_t EXP_ELSE_LEN = 2;

// words 3
// strings | close braces 2
// bool | int 1

int8_t default_expression(
  struct validator_frame_t *validator,
  enum onk_lexicon_t current
){ 
  const enum onk_lexicon_t * ret;

  if(is_expect_operand(current))
  {
    validator->slices[0] = EXPR;
    validator->islices[0] = EXPR_LEN;
    validator->nslices = 1;

    switch(current)
    {
      case ONK_WORD_TOKEN:
        validator->slices[1] = EXP_WORD;
        validator->islices[1] = EXP_WORD_LEN;

        validator->slices[2] = ASNOP;
        validator->islices[2] = ASNOP_LEN;
        validator->nslices = 3;
        break;

      case ONK_STRING_LITERAL_TOKEN:
        validator->slices[1] = EXP_STRING;
        validator->islices[1] = EXP_STRING_LEN;
        validator->nslices = 2;
        break;

      default:
        break;
    }
  }

  else if(is_expect_operator(current))
  {
    validator->slices[1] = &EXP_EXPLICIT_WORD;
    validator->islices[1] = 1;
    validator->nslices = 2;
  }

  else if(onk_is_tok_close_brace(current))
  {
    //validator->slices[1] = NEXT_CLOSE_BRACE;
    //validator->islices[1] = NEXT_CLOSE_BRACE_LEN;
    validator->nslices = 2;
  }

  else if(current == ONK_DOT_TOKEN)
  {
    validator->slices[1] = &EXP_EXPLICIT_WORD;
    validator->slices[1] = 1;
    validator->nslices = 2;
  }

  else if (onk_is_tok_block_keyword(current))
  { 
    ret = handle_keyword(current);
    assert(ret != 0);

    validator->slices[1] = ret;
    validator->islices[1] = 1;
    validator->nslices = 2;
  }

  else if(current == ONK_ELSE_TOKEN)
  {
    validator->slices[1] = EXP_ELSE;
    validator->islices[1] = EXP_ELSE_LEN;
    validator->nslices = 2;
  }

  else return 0;
  return 1;
}

/* */
int8_t expect_for_arg(
  struct validator_frame_t *validator,
  enum onk_lexicon_t current
){
  if (current == ONK_WORD_TOKEN)
  {
    validator->slices[0] = &EXP_EXPLICIT_COMMA;
    validator->islices[0] = 1;
    validator->nslices = 1;
  }
  else if(current == ONK_COMMA_TOKEN)
  {
    validator->slices[0] = &EXP_EXPLICIT_WORD;
    validator->islices[0] = 1;
    validator->nslices = 1;
  }
  else return -1;

  return 0;
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
  return ophead == ONK_BRACE_OPEN_TOKEN 
          && gmod == ONK_IMPL_TOKEN
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
    /*           ^-^ */
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
    || (ophead == onk_for_body_op_token
        && onk_is_tok_unit(current));
}

int8_t ctx_paramter_mode(
  struct validator_frame_t *validator,
  enum onk_lexicon_t current
){
  switch(current) {
    case ONK_WORD_TOKEN:
      validator->slices[0] = &EXP_EXPLICIT_EQL;
      validator->islices[0] = 1;
      validator->nslices = 1;
      break;

    case ONK_COMMA_TOKEN:
      validator->slices[0] = &EXP_EXPLICIT_WORD;
      validator->islices[0] = 1;
      validator->nslices = 1;
      break;

    case ONK_EQUAL_TOKEN:
      validator->slices[0] = EXPR;
      validator->islices[0] = EXPR_LEN;
      validator->nslices = 1;
      break;

    default: return -1;
  }

  return 0;
}

bool start_parameter_mode(
  enum onk_lexicon_t ophead,
  enum onk_lexicon_t current
){
  return (ophead == onk_defsig_op_token && current == ONK_PARAM_OPEN_TOKEN)
    || ((ophead == ONK_STRUCT_TOKEN
         || ophead == onk_struct_init_op_token)
         && current == ONK_BRACE_OPEN_TOKEN);
}

bool use_parameter_mode(enum onk_lexicon_t gmod)
{
  return gmod == onk_defsig_op_token
    || gmod == ONK_STRUCT_TOKEN
    || gmod == onk_struct_init_op_token;
}

/*
** special conditions inside of groups
**
*/
int8_t apply_group_rules(struct validator_frame_t *validator, struct onk_parser_state_t*state)
{
  struct onk_parse_group_t *ghead = group_head(state);
  struct onk_token_t *gmod = group_modifier(state, ghead);
  
  enum onk_lexicon_t current = current_token(state)->type;
  enum onk_lexicon_t ophead = op_head(state)->type;

  // if (gmod > 0)
  //   group_op = ->origin
  
  /*
   * def sig(word=?expr, ..),
   * struct definition {word=?expr, ..},
   *
   * definition {word,  ...}
   * definition {word= expr, ..}
   *                 ^
  */
  if (use_parameter_mode(current))
  {
    switch(current) {
      case ONK_WORD_TOKEN:
        validator->slices[0] = &EXP_EXPLICIT_EQL;
        validator->islices[0] = 1;
        validator->nslices = 1;
        break;

      case ONK_COMMA_TOKEN:
        validator->slices[0] = &EXP_EXPLICIT_WORD;
        validator->islices[0] = 1;
        validator->nslices = 1;
        break;

      case ONK_EQUAL_TOKEN:
        validator->slices[0] = EXPR;
        validator->islices[0] = EXPR_LEN;
        validator->nslices = 1;
        break;

      default: return 1;
    }
    //assert(ctx_paramter_mode(validator, current) == 0);
    return 0;
  }

  /*     def func(word) */
  /*  struct foo {word} */
  /* init_struct {word} */
  /*              ^     */
  else if(start_parameter_mode(ophead, current))
  {
    validator->slices[0] = &EXP_EXPLICIT_WORD;
    validator->nslices = 1;
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
    validator->slices[0] = &EXP_EXPLICIT_OPEN_BRACE;
    validator->nslices = 1;
    return 1;
  }

  else if(gmod)
  {
      if(is_inside_for_args(ophead, gmod->type))
      {
          expect_for_arg(validator, current);
          return 1;
      }

      /* impl word { def foo() {...} }*/
      /*             ^                */
      else if(is_inside_impl_body(ophead, gmod->type, current))
      {
          validator->slices[0] = &EXP_EXPLICIT_OPEN_BRACE; // Probably should be word?
          validator->nslices = 1;
          return 1;
      }
  }

  return 0;
}

/*
** deep clone data into *arr
*/

uint16_t onk_semantic_compile(
  enum onk_lexicon_t *arr,
  struct validator_frame_t *validator
){

  uint16_t slice_sz = 0;
  uint16_t total = 0;
  onk_usize offset = 0;
  //onk_usize inc = 0;

  uint8_t slice_len = 0;
  uint16_t i = 0;

  for (i=0; _ONK_SEM_CHK_SZ > i; i++)
  {
    slice_len = validator->islices[i];
    slice_sz = sizeof(enum onk_lexicon_t) * slice_len;
    
    if(total + slice_len > _ONK_SEM_CHK_SZ)
      return -1;

    total += slice_len;

    assert(memcpy(
      (onk_usize)arr + offset,
      validator->slices[i], slice_sz) != 0);

    offset += slice_sz;
  }

  return total;
}

void onk_semantic_build_frame(
  struct validator_frame_t *validator,
  struct onk_parser_state_t*state
){
  enum onk_lexicon_t current = current_token(state)->type;
  enum onk_lexicon_t ophead = op_head(state)->type;
  enum onk_lexicon_t delim;

  if(apply_group_rules(validator, state) == 0)
    default_expression(validator, current);

  delim = place_delimiter(state);

  if(delim != 0)
  {
    validator->slices[validator->nslices] = delim;
    validator->nslices += 1;
  }

  /* add keywords */
  if (ophead == ONK_BRACKET_OPEN_TOKEN)
  {
    validator->slices[validator->nslices] = KWORD_BLOCK;
    validator->islices[validator->nslices] = KWORD_BLOCK_LEN;
    validator->nslices += 1;
  }
}

