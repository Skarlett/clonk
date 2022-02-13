/*
**
**
*/

#include <stdint.h>
#include <string.h>
#include "lexer/lexer.h"
#include "private.h"

/* TODO: Rules for `FOR`, `WHILE`, `IF` must follow an `(` */
/* TODO: `IF`, `WHILE` signatures cannot */
/*       contain delimiters, or keywords */
/* TODO: ensure the following constrains on struct init */
/*      WORD { WORD=.., } */
/* TODO: ensure the correct delimiter */
/*      is choosen for the current group */
/* TODO: allow for `{x;;}` but not `[x,,]` */
/* TODO: ensure when `ELSE` can be used */
/* TODO: limit delimiters in index access >3 */
/* TODO: import paths only accept WORD/DOT until delim */

/* NOTE: DOT token is excluded here because */
/* because it can't be applied to integers */
/* unlike the other operators which can be applied */
/* to groupings, words & integers */
#define _EX_BIN_OPERATOR \
    ADD, MUL, SUB, DIV, POW, MOD,        \
    PIPE, AMPER, OR, AND,                \
    LT, LTEQ, SHL,                       \
    GT, GTEQ, SHR                        \
    //MINUSEQ, PLUSEQ
    // TILDE, NOT
#define _EX_BIN_OPERATOR_LEN 16


#define _EX_UNARY_OPERATOR TILDE, NOT
#define _EX_UNARY_OPERATOR_LEN 2

#define _EX_DELIM COMMA, COLON, SEMICOLON
#define _EX_DELIM_LEN 3

#define _EX_ASN_OPERATOR \
    EQUAL, PLUSEQ, MINUSEQ, \
    BANDEQL, BOREQL, BNEQL
#define EX_ASN_OPERATOR_LEN 6

#define _EX_OPEN_BRACE PARAM_OPEN, BRACE_OPEN, BRACKET_OPEN
#define _EX_CLOSE_BRACE PARAM_CLOSE, BRACE_CLOSE, BRACKET_CLOSE
#define _EX_BRACE_LEN 3

#define _EX_DATA STRING_LITERAL, WORD, INTEGER
#define _EX_DATA_LEN 3

#define _EX_EXPR \
  _EX_DATA,                                     \
  _EX_OPEN_BRACE,                               \
  _EX_UNARY_OPERATOR

#define _EX_EXPR_LEN                            \
  _EX_DATA_LEN                                  \
  + _EX_BRACE_LEN                               \
  + _EX_UNARY_OPERATOR_LEN


/* NOTE:
 * ELSE is not included,
 * because it needs special checks
*/
#define _EX_KEYWORD                             \
    IF, FUNC_DEF, FROM,                         \
    IMPORT, STRUCT, FOR, WHILE, RETURN

#define _EX_KEYWORD_LEN 8

#define _PV_DEFAULT _EX_EXPR
#define _PV_DEFAULT_LEN _EX_EXPR_LEN

#define _PV_INT                                 \
  _EX_BIN_OPERATOR,                             \
  _EX_CLOSE_BRACE,                              \
  _EX_DELIM

#define _PV_INT_LEN                             \
  _EX_BIN_OPERATOR_LEN                          \
  + _EX_BRACE_LEN                               \
  + _EX_DELIM_LEN

#define _PV_WORD                                \
  _EX_BIN_OPERATOR,                             \
  _EX_ASN_OPERATOR,                             \
  _EX_DELIM,                                    \
  _EX_CLOSE_BRACE,                              \
  PARAM_OPEN,                                   \
  BRACKET_OPEN,                                 \
  DOT

#define _PV_WORD_LEN                            \
  _EX_BIN_OPERATOR_LEN                          \
  + _EX_ASN_OPERATOR_LEN                        \
  + _EX_DELIM_LEN                               \
  + _EX_BRACE_LEN                               \
  + 3

#define _PV_STR                                 \
  _EX_BIN_OPERATOR,                             \
  _EX_CLOSE_BRACE,                              \
  _EX_DELIM,                                    \
  BRACKET_OPEN,                                 \
  DOT

#define _PV_STR_LEN                             \
  _EX_BIN_OPERATOR_LEN                          \
  + _EX_BRACE_LEN                               \
  + _EX_DELIM_LEN                               \
  + 2

#define _PV_DOT WORD
#define _PV_DOT_LEN 1

#define _PV_CLOSE_PARAM                         \
  _EX_BIN_OPERATOR,                             \
  _EX_CLOSE_BRACE,                              \
  _EX_DELIM,                                    \
  DOT,                                          \
  BRACKET_OPEN,                                 \
  PARAM_OPEN

#define _PV_CLOSE_PARAM_LEN                     \
  _EX_BIN_OPERATOR                              \
  + _EX_BRACE_LEN                               \
  + _EX_DELIM_LEN                               \
  + 3

/* throw unexpected token */
int8_t is_illegal_delimiter(const struct Parser *state) {
  struct Group *ghead = group_head(state);
  const struct Token *gmod = group_modifier(state, ghead);

  //TODO: add to predict.c
  //if (ghead->type != CodeBlock)
  //{
  //  if(is_delimiter(next->type))
  //    return -1;
  //}

  return \
    gmod->type == WhileCond
    || gmod->type == IfCond
    || (ghead->type == _IdxAccess && ghead->delimiter_cnt > 2)
    /* check for correct delimiter*/
    // TODO: Handle in predict.c
    //|| (gmod->type == _IdxAccess && current->type != COLON)
    //|| ((ghead->type == TupleGroup || ghead->type == ListGroup) && current->type != COMMA)
    //|| (ghead->type == CodeBlock && current->type != SEMICOLON)
    ;
}

//TODO probably deserves its own mode
/* static enum Lexicon _PV_if[] = { */
/*   PARAM_OPEN, */
/*   0 */
/* } */;

/* #define _PV_if_len 2 */

//#define _PV_KEYWORD _EX_KEYWORD
//#define _PV_KEYWORD_LEN _EX_KEYWORD_LEN

enum Lexicon _PV_import_init[] = {
  WORD,
  DOT,
  0
};
#define _PV_import_init_len 3


enum Lexicon _PV_import_word[] = {
  DOT,
  COMMA,
  SEMICOLON,
  0
};
#define _PV_import_word_len 4

/* null terminated */
uint16_t lex_arr_len(enum Lexicon *arr)
{
  uint16_t i=0;
  for (i=0 ;; i++)
    if(arr[i] == 0)
      break;
  return i;
}

bool can_use_else(enum Lexicon output_head) {
  return output_head == IfBody;
}

bool can_use_keyword(enum Lexicon op_head) {
  return is_open_brace(op_head);
}

/*
 * Write `_EX_EXPR` into state->expecting 
 */
void init_expect_buffer(struct Previsioner *state)
{
    memcpy(state->buffer, _PV_default, sizeof(enum Lexicon) * 8);
    state->buffer[9] = 0;
    /* cast removes cc warning */
    state->data.default_mode.ref = (enum Lexicon *)&state->buffer;
    state->mode = PV_Default; 
}

bool can_addon_keywords(struct Parser *state)
{
  const struct Token *token;
  struct Group *ghead = group_head(state);

  if(ghead->operator_idx > 0)
    token = state->operator_stack[ghead->operator_idx-1];

  /* if open brace */
  if(is_open_brace(ophead->type))
    return  == 0;

  return true;
}

uint8_t prevision_keywords(enum Lexicon *buf, struct Expr *expr_head) {
  uint8_t offset = _PV_kw_len - 1;

  memcpy(buf, _PV_kw, sizeof(enum Lexicon) * offset);

  /* if(expr_head->type == IfExprT) */
  /* { */
  /*   buf[offset] = ELSE; */
  /*   offset += 1; */
  /* } */

  buf[offset] = 0;
  offset += 1;
  
  return offset;
}

/* 
 * @param ref NULL terminated  
 */
int8_t prevision_next(struct Parser *state)
{
  enum Lexicon current = state->src[*state->_i].type;
  enum Lexicon *ref = 0;

  uint16_t offset = 0;

  switch (current)
  {
    case WORD:
      offset = _PV_word_len - 1;
      ref =  _PV_word;
      break;
    
    case INTEGER:
      offset = _PV_int_len - 1;
      ref = _PV_int;
      break;
    
    case STRING_LITERAL:
      offset = _PV_str_len - 1;
      ref = _PV_str;
      break;
    
    case DOT:
      offset = 1;
      state->expecting.buffer[0] = WORD;
      state->expecting.buffer[1] = 0;
      break;
    
    case IF:
      //TODO make mode
      offset = _PV_if_len - 1;
      ref = _PV_if; 
      break;
    
    case ELSE:
      offset = _PV_default_len - 1;
      ref = _PV_default;
      break;
    
    case RETURN:
      offset = _PV_default_len - 1;
      ref = _PV_default;
      break;

    case FUNC_DEF:
      offset = 1;
      state->expecting.buffer[0] = WORD;
      state->expecting.buffer[1] = 0;
      state->expecting.mode = PV_DefSignature;
      break;

    case IMPORT:
      offset = _PV_import_init_len  - 1;
      ref = _PV_import_init;
      state->expecting.mode = PV_Import; 
      break;

    default:
      if (is_operator(current))
        ref = _PV_default;

      else if (is_delimiter(current))
        ref = _PV_default;

      /* any open brace */
      else if (is_open_brace(current))
      { 
        memcpy(state->expecting.buffer, _PV_default, sizeof(enum Lexicon) * (_PV_default_len - 1));
        /* add opposite brace type to expectation */
        state->expecting.buffer[8] = invert_brace_tok_ty(current);
        ref = (enum Lexicon *)&state->expecting.buffer;
      }
      /* any open brace */
      else if (is_close_brace(current)) 
        ref = exp_close_param;

      else
        return -1;
  }

  if (ref) {
    memcpy(state->expecting.buffer, ref, sizeof(enum Lexicon) * offset); 
  }

  if(can_addon_keywords((op_head(state)->type == IfCond)))
  {
    memcpy(state->expecting.buffer + sizeof(enum Lexicon) * offset, _PV_kw, _PV_kw_len - 1);
    offset += _PV_kw_len - 1;
  }
  // TODO: use operator stack head instead

  if(state->operators_ctr > 0 && op_head(state)->type == IfCond)
  {
    state->expecting.buffer[offset + 1] = ELSE;
    offset += 1; 
  }

  state->expecting.buffer[offset + 1] = 0;

  return 0;
}

/* setup delimiter expectation */
/* enum Lexicon get_expected_delimiter(struct Group *ghead) */
/* {   */
/*   if(ghead->state & GSTATE_CTX_CODE_GRP) */
/*     return SEMICOLON; */

/*   else if(ghead->state & GSTATE_CTX_DATA_GRP) */
/*     return COMMA; */

/*   else if (ghead->state & GSTATE_CTX_IDX) */
/*     return COLON; */

/*   else if (ghead->state & GSTATE_CTX_MAP_GRP) */
/*   { */
/*     if (ghead->delimiter_cnt % 2 == 0) */
/*       return COMMA; */
/*     else */
/*       return COLON; */
/*   } */
/*   else */
/*     return 0; */
/* } */

enum ModeResult {
  _MRMatchFailure = 0,
  _MRContinue = 1,
  _MRComplete = 2,
  _MRError = -1
};

enum ModeResult mode_func_def(enum Lexicon current, struct Previsioner *state)
{
  enum ModeResult ret = _MRMatchFailure;
  uint16_t mod = 0;

  switch(state->data.fndef_mode.ctr)
  {
    case 0:
      return current == WORD;
    case 1:
      return current == PARAM_OPEN;
    default:
      mod = state->data.fndef_mode.ctr % 2;

      if (current == PARAM_CLOSE)
        return _MRComplete;
      
      return (current == WORD && mod) 
        || (current == COMMA && !mod);
  }

  return ret;
}

enum ModeResult mode_import(enum Lexicon current,  struct Previsioner *state)
{

  if(!state->data.import_mode.has_word){
    memcpy(state->buffer, _PV_import_init, _PV_import_word_len);
    return 1;
  }
    
  else if (current == WORD)
  {
    memcpy(state->buffer, _PV_import_word, _PV_import_word_len);
    return 1;
  }
  
  else if(current == DOT || current == COMMA) {
    state->buffer[0] = WORD;
    state->buffer[1] = 0;
  }

  else if(current == SEMICOLON)
    return 2;

  else return -1;
  return -1;
}

int8_t mode_default(enum Lexicon current, enum Lexicon grp_delim, struct Previsioner *expecting)
{
  /* check previous expecting buffer */
  if (!eq_any_tok(current, expecting->data.default_mode.ref))
    return -1;

  /* if current is delimiter, is correct delimiter? */
  else if(is_delimiter(current) && grp_delim != current)
    return -1;
  
  return 0;
}

int8_t is_token_unexpected(struct Parser *state)
{
  struct Token *current = &state->src[*state->_i];
  struct Group *ghead = 0;
  int8_t mode_ret = 0;
  enum Lexicon delim = 0;
  
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
  //else if(!eq_any_tok(SEMICOLON, state->expecting.exp_ref))
    /* incomplete expression, must get another token */
   // state->panic_flags |= STATE_INCOMPLETE;
  //else 
  //  state->panic_flags &= (uint16_t)STATE_INCOMPLETE;

  /* setup next token's expectation */
  return prevision_next(state);
}
