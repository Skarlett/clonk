/*
**
**
*/
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "lexer/lexer.h"
#include "private.h"

/* TODO: Rules for `FOR`, `WHILE`, `IF` must follow an `(` */
/* TODO: `IF`, `WHILE` signatures cannot */
/*       contain delimiters, or keywords */
/* TODO: ensure the following constrains on struct init */
/*      ONK_WORD_TOKEN { ONK_WORD_TOKEN=[expr], } */
/* TODO: ensure the correct delimiter */
/*      is choosen for the current group */
/* TODO: allow for `{x;;}` but not `[x,,]` */
/* TODO: ensure when `ELSE` can be used */
/* TODO: limit delimiters in index access 3 >= */
/* TODO: import paths only accept ONK_WORD_TOKEN/ONK_DOT_TOKEN until delim */
/* TODO: figure out if you can declare a new variable */

/* because it can't be applied to integers */
/* unlike the other operators which can be applied */
/* to groupings, words & integers */

#define _EX_BIN_OPERATOR                 \
    ONK_ADD_TOKEN, ONK_MUL_TOKEN, ONK_SUB_TOKEN, ONK_DIV_TOKEN, ONK_POW_TOKEN, ONK_MOD_TOKEN,        \
    PIPE, AMPER, OR, AND,                \
    LT, LTEQ, SHL,                       \
    GT, GTEQ, SHR                        \
    //MINUSEQ, PLUSEQ
    // ONK_TILDE_TOKEN, ONK_NOT_TOKEN
#define _EX_BIN_OPERATOR_LEN 16


#define _EX_UNARY_OPERATOR ONK_TILDE_TOKEN, ONK_NOT_TOKEN
#define _EX_UNARY_OPERATOR_LEN 2

#define _EX_DELIM ONK_COMMA_TOKEN, ONK_COLON_TOKEN, ONK_SEMICOLON_TOKEN
#define _EX_DELIM_LEN 3

#define _EX_ASN_OPERATOR \
    EQUAL, PLUSEQ, MINUSEQ, \
    BANDEQL, BOREQL, BNEQL
#define _EX_ASN_OPERATOR_LEN 6

#define _EX_OPEN_BRACE ONK_PARAM_OPEN_TOKEN, ONK_BRACE_OPEN_TOKEN, ONK_BRACKET_OPEN_TOKEN
#define _EX_CLOSE_BRACE ONK_PARAM_CLOSE_TOKEN, ONK_BRACE_CLOSE_TOKEN, ONK_BRACKET_CLOSE_TOKEN
#define _EX_BRACE_LEN 3

#define _EX_DATA ONK_STRING_LITERAL_TOKEN, ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_KEYWORD_TOKEN
#define _EX_DATA_LEN 3

#define _EX_EXPR_LIMITED                        \
  _EX_UNARY_OPERATOR,                           \
  _EX_DATA

#define _EX_EXPR_LIMITED_LEN                    \
  _EX_DATA_LEN                                  \
  + _EX_UNARY_OPERATOR_LEN

#define _EX_EXPR \
  _EX_EXPR_LIMITED,                             \
  _EX_OPEN_BRACE,

#define _EX_EXPR_LEN                            \
  _EX_EXPR_LIMITED_LEN                           \
  + _EX_BRACE_LEN

 * ELSE is not included,
 * because it needs special checks
*/
#define _EX_KEYONK_WORD_TOKEN                             \
    IF, FUNC_DEF, FROM,                         \
    IMPORT, STRUCT, FOR, WHILE, RETURN

#define _EX_KEYONK_WORD_TOKEN_LEN 8

#define _PV_DEFAULT _EX_EXPR
#define _PV_DEFAULT_LEN _EX_EXPR_LEN

#define _PV_INT                                 \
  _EX_BIN_OPERATOR,                             \
  _EX_CLOSE_BRACE,                              \
//  _EX_DELIM

#define PV_INT_LEN                             \
  _EX_BIN_OPERATOR_LEN                          \
  + _EX_BRACE_LEN                               \
//  + _EX_DELIM_LEN

const enum onk_lexicon_t PV_INT[] = {_PV_INT};

#define _PV_ONK_WORD_TOKEN                                \
  _EX_BIN_OPERATOR,                             \
  _EX_ASN_OPERATOR,                             \
  _EX_CLOSE_BRACE,                              \
  ONK_PARAM_OPEN_TOKEN,                                   \
  ONK_BRACKET_OPEN_TOKEN,                                 \
  ONK_DOT_TOKEN
//  _EX_DELIM,

#define PV_ONK_WORD_TOKEN_LEN                             \
  _EX_BIN_OPERATOR_LEN                          \
  + _EX_ASN_OPERATOR_LEN                        \
  + _EX_BRACE_LEN                               \
  + 3
//  + _EX_LEN_DELIM                               \

const enum onk_lexicon_t PV_ONK_WORD_TOKEN[] = {_PV_ONK_WORD_TOKEN};

#define _PV_STR                                 \
  _EX_BIN_OPERATOR,                             \
  _EX_CLOSE_BRACE,                              \
  ONK_BRACKET_OPEN_TOKEN,                                 \
  ONK_DOT_TOKEN
//  _EX_DELIM,                                    \

#define PV_STR_LEN                             \
  _EX_BIN_OPERATOR_LEN                          \
  + _EX_BRACE_LEN                               \
  + 2
//  + _EX_DELIM_LEN
const enum onk_lexicon_t PV_STR[] = {_PV_STR};

//
#define _PV_ONK_DOT_TOKEN ONK_WORD_TOKEN
#define _PV_ONK_DOT_TOKEN_LEN 1

#define _PV_CLOSE_PARAM                         \
  _EX_BIN_OPERATOR,                             \
  _EX_CLOSE_BRACE,                              \
  ONK_DOT_TOKEN,                                          \
  ONK_BRACKET_OPEN_TOKEN,                                 \
  ONK_PARAM_OPEN_TOKEN
//  _EX_DELIM,

const enum onk_lexicon_t PV_CLOSE_BRACE[] = {_PV_CLOSE_PARAM};

#define PV_CLOSE_BRACE_LEN                      \
  _EX_BIN_OPERATOR                              \
  + _EX_BRACE_LEN                               \
  + 3
//  + _EX_DELIM_LEN

const enum onk_lexicon_t PV_DEFAULT[] = {_EX_EXPR};

const enum onk_lexicon_t PV_LIMITED[] = {_EX_EXPR_LIMITED};

int8_t fill_buffer(
  enum onk_lexicon_t current,
  enum onk_lexicon_t *buf,
  uint16_t buf_sz
){

  enum onk_lexicon_t *selected = 0;
  enum onk_lexicon_t small[8];
  uint16_t nitems = 0;
  bool allow_delim = 0;

  enum PrevisionerModeT mode = default_mode_t;

  if(onk_is_tok_operator(current) || onk_is_tok_delimiter(current) || current == IN)
  {
    selected = (enum onk_lexicon_t *)PV_DEFAULT;
    nitems = _EX_EXPR_LEN;
  }

  else if (
    current == IF
    || current == FOR
    || current == WHILE)
  {
    small[0] = ONK_PARAM_OPEN_TOKEN;
    nitems = 1;
  }

  else if (current == FUNC_DEF
    || current == STRUCT
    || current == IMPL
    || current == FOR)
  {
    small[0] = ONK_WORD_TOKEN;
    nitems = 1;
  }

  else if(current == FROM )
  {
    small[0] = ONK_FROM_LOCATION_TOKEN;
    nitems = 1;
  }

  else if (onk_is_tok_open_brace(current))
  {

    /* add opposite brace type to expectation */
    selected = (enum onk_lexicon_t *)PV_DEFAULT;
    nitems = _EX_EXPR_LEN;

    //TODO:
    //selected[nitems] = invert_brace_tok_ty(current);
    //nitems += 1;

  }
  /* any open brace */
  else if (onk_is_tok_close_brace(current))
  {
    allow_delim = 1;
    selected = (enum onk_lexicon_t *)PV_CLOSE_BRACE;
    nitems = PV_CLOSE_BRACE_LEN;
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

    case FUNC_DEF:
      nitems = 1;
      small[0] = ONK_WORD_TOKEN;
      break;

    case IMPORT:
        mode =
      //nitems = PV_I_init_len  - 1;
      //selected = _PV_import_init;
      //state->expecting.mode = PV_Import;
      break;

    default:
      break;
  }

  /* any open brace */
  else
    return -1;

  if (selected)
    assert(memcpy(
      buf, selected,
      sizeof(enum onk_lexicon_t) * nitems
    ));


  if(can_addon_keywords((op_head(state)->type == IfCond)))
  {
    memcpy(state->expecting.buffer + sizeof(enum onk_lexicon_t) * nitems, _PV_kw, _PV_kw_len - 1);
    nitems += _PV_kw_len - 1;
  }
  // TODO: use operator stack head instead

  if(state->operators_ctr > 0 && op_head(state)->type == IfCond)
  {
    state->expecting.buffer[nitems + 1] = ELSE;
    nitems += 1;
  }

  state->expecting.buffer[nitems + 1] = 0;

  return 0;
  



  state->buffer
}


/* null terminated */
uint16_t lex_arr_len(enum onk_lexicon_t *arr)
{
  uint16_t i=0;
  for (i=0 ;; i++)
    if(arr[i] == 0)
      break;
  return i;
}

bool can_use_else(enum onk_lexicon_t output_head){
  return output_head == IfBody;
}


void place_delimiter(struct Parser *state)
{
  struct Group *ghead = group_head(state);
  const struct onk_token_t *gmod = group_modifier(state, ghead);
  struct Previsioner *previsioner = &state->expecting;
  enum onk_lexicon_t *buf = previsioner->buffer;

  /* bool single_check = buf_ctr+1 > PREVISION_SZ; */
  /* bool dual_check = buf_ctr+2 > PREVISION_SZ; */

  if (gmod->type == WhileCond || gmod->type == IfCond)
    return;

  else if(ghead->type == _IdxAccess && ghead->delimiter_cnt > 2)
     return; // give ] or expr

  else if (ghead->type == onk_partial_brace_group_token)
  {
    previsioner->buffer[previsioner->buf_ctr] = ONK_SEMICOLON_TOKEN;
    previsioner->buffer[previsioner->buf_ctr + 1] = ONK_COLON_TOKEN;
    previsioner->buf_ctr += 2;
    return;
  }

  if (ghead->type == onk_list_group_token
      || ghead->type == onk_struct_group_token
      || ghead->type == onk_tuple_group_token)
      previsioner->buffer[previsioner->buf_ctr] = ONK_COMMA_TOKEN;

  else if (ghead->type == onk_map_group_token)
  {
    if(ghead->delimiter_cnt % 2 == 0)
       previsioner->buffer[previsioner->buf_ctr] = ONK_COLON_TOKEN;
    else
       previsioner->buffer[previsioner->buf_ctr] = ONK_COMMA_TOKEN;
  }

  else if (ghead->type == onk_code_group_token)
    previsioner->buffer[previsioner->buf_ctr] = ONK_SEMICOLON_TOKEN;

  else if(ghead->type == onk_idx_group_token)
    previsioner->buffer[previsioner->buf_ctr] = ONK_COLON_TOKEN;

  previsioner->buf_ctr += 1;
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

bool can_use_keywords(struct Parser *state)
{
  struct Group *ghead = group_head(state);
  const struct onk_token_t *gmod = group_modifier(state, ghead);
  const struct onk_token_t *ophead = ophead(state);

  if(!onk_is_tok_open_brace(ophead->type))
    return false;

  else if(ghead->operator_idx > 0 && onk_is_tok_group_modifier(gmod->type))
    return false;

  return true;
}

uint8_t prevision_keywords(enum onk_lexicon_t *buf) {
  uint8_t nitems = _PV_kw_len - 1;

  memcpy(buf, _PV_kw, sizeof(enum onk_lexicon_t) * nitems);

  /* if(expr_head->type == IfExprT) */
  /* { */
  /*   buf[nitems] = ELSE; */
  /*   nitems += 1; */
  /* } */


  return nitems;
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
