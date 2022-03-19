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

/* */
/* bool is_expecting_operator(enum onk_lexicon_t current) */
/* { */
/*   return onk_is_tok_unit(current); */
/* } */
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


void terminator(struct Parser *state)
{
  place_closing_brace(state);
  place_delimiter(state);
}


/* TODO: Rules for `ONK_FOR_TOKEN`, `ONK_WHILE_TOKEN`, `ONK_IF_TOKEN` must follow an `(` */
/* TODO: `ONK_IF_TOKEN`, `ONK_WHILE_TOKEN` signatures cannot */
/*       contain delimiters, or keywords */
/* TODO: ensure the following constrains on struct init */
/*      ONK_WORD_TOKEN { ONK_WORD_TOKEN=[expr], } */
/* TODO: ensure the correct delimiter */
/*      is choosen for the current group */
/* TODO: allow for `{x;;}` but not `[x,,]` */
/* TODO: ensure when `ONK_ELSE_TOKEN` can be used */
/* TODO: limit delimiters in index access 3 >= */
/* TODO: import paths only accept ONK_WORD_TOKEN/ONK_DOT_TOKEN until delim */
/* TODO: figure out if you can declare a new variable */

/* `struct/def/impl` expects an explicit word/ident after its occurance */
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

uint8_t expect_strict_seq(enum onk_lexicon_t current)
{
  return _explicit_expecting_word(current)
    || _explicit_expecting_open_param(current);
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

  if(apply_group_rules(validator, state) == 0)
    default_expression(validator, current);

  place_delimiter(validator, state);

  if (ophead == ONK_BRACKET_OPEN_TOKEN)
  {
    validator->slices[validator->nslices] = (enum onk_lexicon_t *)BLOCK_KWORD;
    validator->islices[validator->nslices] = KWORD_BLOCK_LEN;
    validator->nslices = 1;
  }

  return 0;
}

int8_t fill_buffer(
  struct validator_t *validator,
  struct Parser *state
){

  next_frame(validator, state);

  /* any open brace */
  if (validator->slices)
    assert(memcpy(
      buf, validator->slices,
      sizeof(enum onk_lexicon_t) * nitems
    ));


  if(can_addon_keywords((op_head(state)->type == onk_ifcond_op_token)))
  {
    memcpy(state->expecting.buffer + sizeof(enum onk_lexicon_t) * nitems, _PV_kw, _PV_kw_len - 1);
    nitems += _PV_kw_len - 1;
  }


  // TODO: use operator stack head instead
  if(state->operators_ctr > 0 && op_head(state)->type == onk_ifcond_op_token)
  {
    state->expecting.buffer[nitems + 1] = ONK_ELSE_TOKEN;
    nitems += 1;
  }

  state->expecting.buffer[nitems + 1] = 0;

  return 0;


  state->buffer
}


//TODO probably deserves its own mode
/* static enum onk_lexicon_t _PV_if[] = { */
/*   ONK_PARAM_OPEN_TOKEN, */
/*   0 */
/* } */;

/* #define _PV_if_len 2 */

//#define _PV_KEYONK_WORD_TOKEN _EX_KEYONK_WORD_TOKEN
//#define _PV_KEYONK_WORD_TOKEN_LEN _EX_KEYONK_WORD_TOKEN_LEN

/*
 * Write `_EX_EXPR` into state->expecting
 */
void init_expect_buffer(struct Previsioner *state)
{
    memcpy(state->buffer, _PV_default, sizeof(enum onk_lexicon_t) * 8);
    state->buffer[9] = 0;
    /* cast removes cc warning */
    state->data.default_mode.validator->slices = (enum onk_lexicon_t *)&state->buffer;
    state->mode = PV_Default; 
}

/* 
 * @param validator->slices NULL terminated
 */
int8_t prevision_next(struct Parser *state)
{
  enum onk_lexicon_t current = state->src[*state->_i].type;

}

enum ModeResult {
  _MRMatchFailure = 0,
  _MRContinue = 1,
  _MRComplete = 2,
  _MRError = -1
};

enum ModeResult mode_func_def(enum onk_lexicon_t current, struct Previsioner *state)
{
  enum ModeResult ret = _MRMatchFailure;
  uint16_t mod = 0;

  switch(state->data.fndef_mode.ctr)
  {
    case 0:
      return current == ONK_WORD_TOKEN;
    case 1:
      return current == ONK_PARAM_OPEN_TOKEN;
    default:
      mod = state->data.fndef_mode.ctr % 2;

      if (current == ONK_PARAM_CLOSE_TOKEN)
        return _MRComplete;
      
      return (current == ONK_WORD_TOKEN && mod) 
        || (current == ONK_COMMA_TOKEN && !mod);
  }

  return ret;
}

enum ModeResult mode_import(enum onk_lexicon_t current,  struct Previsioner *state)
{

  if(!state->data.import_mode.has_word){
    memcpy(state->buffer, _PV_import_init, _PV_import_word_len);
    return 1;
  }
    
  else if (current == ONK_WORD_TOKEN)
  {
    memcpy(state->buffer, _PV_import_word, _PV_import_word_len);
    return 1;
  }
  
  else if(current == ONK_DOT_TOKEN || current == ONK_COMMA_TOKEN) {
    state->buffer[0] = ONK_WORD_TOKEN;
    state->buffer[1] = 0;
  }

  else if(current == ONK_SEMICOLON_TOKEN)
    return 2;

  else return -1;
  return -1;
}

int8_t mode_default(enum onk_lexicon_t current, enum onk_lexicon_t grp_delim, struct Previsioner *expecting)
{
  /* check previous expecting buffer */
  if (!onk_eq_any_tok(current, expecting->data.default_mode.validator->slices))
    return -1;

  /* if current is delimiter, is correct delimiter? */
  else if(onk_is_tok_delimiter(current) && grp_delim != current)
    return -1;
  
  return 0;
}


int8_t is_token_unexpected(struct Parser *state)
{
  struct onk_token_t *current = &state->src[*state->_i];
  struct Group *ghead = 0;
  int8_t mode_ret = 0;
  enum onk_lexicon_t delim = 0;
  
  if(state->expecting.mode == PV_Default
     && mode_default(current->type, get_expected_delimiter(ghead), &state->expecting) == -1)
     return -1;
  
  else if(state->expecting.mode == PV_DefSignature)
  {
    mode_ret = mode_func_def(current->type, &state->expecting);
    
    if(mode_ret == 0)
      return true;
    
    else if(mode_ret == 1)
      return false;
    
    else if(mode_ret == 2)
    {
      state->expecting.mode = PV_Default;
      return false;
    }
    else return -1;
  }

  /* expecting buffers contain all delimiters,
  * so we may check any */
  //else if(!onk_eq_any_tok(ONK_SEMICOLON_TOKEN, state->expecting.exp_validator->slices))
    /* incomplete expression, must get another token */
   // state->panic_flags |= STATE_INCOMPLETE;
  //else 
  //  state->panic_flags &= (uint16_t)STATE_INCOMPLETE;

  /* setup next token's expectation */
  return prevision_next(state);
}
