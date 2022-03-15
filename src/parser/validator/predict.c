/*
** predict.c
** This file provides functionality for determining validity of parser input.
** It works by looking at its current token and the parser's state,
** then spits out what tokens are possible next.
** If the next token in the input is not contained
** in the what is predicted, then it is invalid input.
** Syntax error or otherwise.
**
*/

#include <assert.h>
#include <stdint.h>
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


bool place_bracket(struct Parser *state)
{
  const struct onk_token_t *current = current_token(state);

  if(current->type == ONK_WORD_TOKEN)
    return 1;

  else if(current->type == ONK_STRING_LITERAL_TOKEN)
    return 1;

  else if(onk_is_tok_close_brace(current->type))
    return 1;

  return 0;
}

bool place_param(struct Parser *state)
{
  const struct onk_token_t *current = current_token(state);
  if(current->type == ONK_WORD_TOKEN)
    return 1;

  else if(onk_is_tok_close_brace(current->type))
    return 1;

  return 0;
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

int8_t expect_default_expression(enum lexicon_t current)
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
  struct validator_t *state,
  enum onk_lexicon_t current
){

  enum onk_lexicon_t *selected[12];
  enum onk_lexicon_t small[16];

  if(onk_is_tok_unit(current))
  {
    selected[0] = (enum onk_lexicon_t *)&EXPR;
    switch(current)
    {
      case ONK_WORD_TOKEN:
        small[0] = ONK_DOT_TOKEN;
        small[1] = ONK_BRACKET_OPEN_TOKEN;
        small[2] = ONK_PARAM_OPEN_TOKEN;
        small[3] = ONK_BRACE_OPEN_TOKEN;
        break;

      case ONK_STRING_LITERAL_TOKEN:
        small[0] = ONK_DOT_TOKEN;
        small[1] = ONK_BRACKET_OPEN_TOKEN;
        break;

      default:
        break;
    }
  }

  else if(onk_is_tok_open_brace(current))
  {
    selected[0] = (enum onk_lexicon_t *)&EXPR;
  }

  else if(onk_is_tok_close_brace(current))
  {
    selected[0] = (enum onk_lexicon_t *)&NEXT_CLOSE_BRACE;
  }

  else if(current == ONK_DOT_TOKEN)
  {
    small[0] = ONK_WORD_TOKEN;
  }

  else if (onk_is_tok_block_keyword(current))
  {
    small[0] = handle_keyword(current);
  }

  /* catch all binops & unary ops */
  /* ASSUME: all keywords were not `current` token type  */
  else if(onk_is_tok_operator(current))
  {
    selected[0] = (enum onk_lexicon_t *)&NEXT_CLOSE_BRACE;
  }

  else if (onk_is_tok_delimiter(current))
  {
    selected[0] = (enum onk_lexicon_t *)&EXPR;
  }

  else if(current == ONK_ELSE_TOKEN)
  {
    small[0] = ONK_IF_TOKEN;
    small[1] = ONK_BRACE_OPEN_TOKEN;
  }

  else return -1;

  return 0;
}

int8_t do_parameter_mode(struct Parser *state)
{

}

int8_t ctx_for_arg_mode(enum onk_lexicon_t current)
{
  if (current == ONK_WORD_TOKEN)
    //small[0] = ONK_COMMA_TOKEN;
  {}
  else if(current == ONK_COMMA_TOKEN)
  {
   small[0] = ONK_WORD_TOKEN;
  }
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
/* use `{` next */
bool start_block(
  enum onk_lexicon_t current,
  enum onk_lexicon_t ophead
)
{
  return
    /*   def sig(){ */
    /* while sig(){ */
    /*  if(foobar){ */
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

int8_t ctx_paramter_mode(enum onk_lexicon_t current)
{
  enum onk_lexicon_t small, *selected;

  if (current == ONK_WORD_TOKEN)
  {
    small = ONK_EQUAL_TOKEN;
  }
  else if (current == ONK_COMMA_TOKEN)
    small = ONK_WORD_TOKEN;

  else if(current == ONK_EQUAL_TOKEN)
    selected = (enum onk_lexicon_t *)&EXPR;

  else
    return -1;
  return 0;
}

bool start_parameter_mode(enum onk_lexicon_t ophead, enum onk_lexicon_t current)
{

  return (ophead == DefSign && current == ONK_PARAM_OPEN_TOKEN)
    || ((ophead == ONK_STRUCT_TOKEN
         || ophead == onk_struct_init_op_token)
         && current == ONK_BRACE_OPEN_TOKEN);
}


int8_t expect_strict_ctx(struct Parser *state, enum onk_lexicon_t current)
{
  enum onk_lexicon_t ophead = op_head(state)->type;
  struct Group *ghead = group_head(state);
  enum onk_lexicon_t gmod = group_modifier(state, ghead)->type;
  enum onk_lexicon_t small[16];
  enum onk_lexicon_t *selected[12];

  /*
   * def sig(word=?expr, ..),
   * struct definition {word=?expr, ..},
   *
   * definition {word,  ...}
   * definition {word= expr, ..}
   *                 ^
  */
  if (use_parameter_mode(current))
      ctx_paramter_mode(current);


  /*     def func(word) { }*/
  /*  struct foo {word}    */
  /* init_struct {word}    */
  /*              ^        */
  else if(start_parameter_mode(ophead, current))
    small[0] = ONK_WORD_TOKEN;

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
    small[0] = ONK_BRACE_OPEN_TOKEN;


  /* TODO: needs `ELSE` to **added** onto the buffer
   * instead of shown as the only option */
  /***************************************/
  /* if (..) { } else */
  /*             ^    */
  /* else if(ophead == onk_ifbody_op_token */
  /*         && current == ONK_BRACE_CLOSE_TOKEN) */
  /*   small[0] = ONK_ELSE_TOKEN; */

  /* special handling for certain groups & modifiers */
  else if(is_inside_for_args(ophead, gmod))
    ctx_for_arg_mode(current);


  /* impl word { def foo() {...} }*/
  /*             ^                */
  else if(is_inside_impl_body(ophead, gmod, current))
    small[0] = ONK_DEF_TOKEN;


  // catch the tail end of the expression
  // and
  if((ophead == onk_ifbody_op_token
     || ophead == onk_while_cond_op_token)
     && current == ONK_PARAM_CLOSE_TOKEN)
  {
    small[0] = ONK_BRACE_OPEN_TOKEN;
  }

  else if(ophead->type == ONK_PARAM_OPEN_TOKEN
          && gmod == DefSign
          && current == ONK_WORD_TOKEN){}

  else if(ophead->)
  {}
  else if() {}
}

int8_t fill_buffer(
  struct validator_t *state,
  enum onk_lexicon_t current,
  enum onk_lexicon_t *buf,
  uint16_t buf_sz
){

  struct ValidatorFrame ctx = validator_frame_head(state);
  enum onk_lexicon_t *selected[0];
  enum onk_lexicon_t small[16];

  uint16_t nitems = 0;
  bool allow_delim = 0;

  enum PrevisionerModeT mode = default_mode_t;

  /*
   * when the current token is labeled as "strict"
   * it will then follow a sequence of tokens after it,
   * any token not matched exactly fails.
  */



  /*
   * *strict*: must follow a sequence of token(s)
   *
   * when the context meets certain conditions,
   * we will expect a sequence of tokens to follow
  */
  else if(expect_strict_ctx_seq(state))
  {

    return 0;
  }


  else if(_onk_do_default_expectation(current))
  {
    default_expression(state, current);

    if(add_tokens_based_on_ctx(state))


    return 0;
  }



  if(add_delimiter)



  if(current == ONK_DEF_TOKEN)
  {}

  else if(current == ONK_FROM_TOKEN )
  {
    small[0] = ONK_FROM_LOCATION;
    nitems = 1;
  }

  else switch (current)
  {
    case ONK_WORD_TOKEN:
      allow_delim = 1;
      nitems = PV_ONK_WORD_TOKEN_LEN;
      selected =  (enum onk_lexicon_t *)PV_ONK_WORD_TOKEN;
      break;

    case ONK_INTEGER_TOKEN:
      allow_delim = 1;
      nitems = PV_INT_LEN;
      selected = (enum onk_lexicon_t *)PV_INT;
      break;

    case ONK_STRING_LITERAL_TOKEN:
      allow_delim = 1;
      nitems = PV_STR_LEN;
      selected = (enum onk_lexicon_t *)PV_STR;
      break;

    case ONK_DOT_TOKEN:
      nitems = 1;
      small[0] = ONK_WORD_TOKEN;
      break;

    case ONK_DEF_TOKEN:
      nitems = 1;
      small[0] = ONK_WORD_TOKEN;
      break;

    case ONK_IMPORT_TOKEN:
        mode = 0;
      //nitems = PV_I_init_len  - 1;
      //selected = _PV_import_init;
      //state->expecting.mode = PV_Import;
      break;

    default:
      return -1;
  }

  /* any open brace */
  if (selected)
    assert(memcpy(
      buf, selected,
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
    state->data.default_mode.selected = (enum onk_lexicon_t *)&state->buffer;
    state->mode = PV_Default; 
}



/* 
 * @param selected NULL terminated
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
  if (!onk_eq_any_tok(current, expecting->data.default_mode.selected))
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
  //else if(!onk_eq_any_tok(ONK_SEMICOLON_TOKEN, state->expecting.exp_selected))
    /* incomplete expression, must get another token */
   // state->panic_flags |= STATE_INCOMPLETE;
  //else 
  //  state->panic_flags &= (uint16_t)STATE_INCOMPLETE;

  /* setup next token's expectation */
  return prevision_next(state);
}
