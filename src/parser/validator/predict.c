
#include <assert.h>
#include <stdint.h>
#include <string.h>

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

/* `struct/def/impl` expects an explicit word/ident after its occurance */
bool is_expecting_word(enum onk_lexicon_t current)
{
  return current == ONK_STRUCT_TOKEN
    || current == ONK_IMPL_TOKEN
    || current == ONK_DEF_TOKEN;
}

/* `if/while` expects an explicit onk_open_param_token after its occurance */
uint8_t is_expecting_open_param(enum onk_lexicon_t current)
{
  return current == ONK_IF_TOKEN
    || current == ONK_WHILE_TOKEN
    || current == ONK_FOR_TOKEN;
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

  /* give nothing back if index access
   * & currently an opening bracket  */
  if(ghead->type == onk_idx_group_token
     && current->type == ONK_BRACKET_OPEN_TOKEN)
     return ONK_UNDEFINED_TOKEN;

  else if(gmod->type == onk_ifcond_op_token
     && current->type == ONK_PARAM_OPEN_TOKEN)
    return ONK_UNDEFINED_TOKEN;

  else if(gmod->type == onk_while_cond_op_token
     && current->type == ONK_PARAM_OPEN_TOKEN)
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

  if(onk_is_tok_open_brace(current)
     || onk_is_tok_delimiter(current)
     || onk_is_tok_operator(current))
  {
    if (onk_is_tok_open_brace(current))
    {
      if(ONK_STACK_SZ <= state->nstack)
        return -1;
      state->nstack += 1;
    }

    selected[0] = (enum onk_lexicon_t *)&EXPR;
    return 0;
  }

  else if(onk_is_tok_close_brace(current)
    || onk_is_tok_unit(current))
  {
    if(onk_is_tok_close_brace(current))
    {
      if(state->nstack == 0)
        return -1;
    }

    state->ref_buffer[0] = (enum onk_lexicon_t *)_NEXT_CLOSE_BRACE;
    state->nstack -= 1;
  }

  else if(current == ONK_DEF_TOKEN)
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
