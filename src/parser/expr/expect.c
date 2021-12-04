#include <stdint.h>
#include <string.h>
#include "../lexer/lexer.h"
#include "expr.h"
#include "utils.h"

#define _EX_BIN_OPERATOR \
    ADD, MUL, SUB, DIV, POW, MOD, \
    ISEQL, ISNEQL, LT, LTEQ, OR, AND,\
    GTEQ, GT, SHL, SHR, AMPER, PIPE, PIPEOP

#define _EX_UNARY_OPERATOR \
    TILDE, NOT

#define _EX_DELIM \
    COMMA, COLON, SEMICOLON

#define _EX_ASN_OPERATOR \
    EQUAL, PLUSEQ, MINUSEQ, \
    BANDEQL, BOREQL, BNEQL

#define _EX_OPEN_BRACE \
    PARAM_OPEN, BRACE_OPEN, BRACKET_OPEN

#define _EX_CLOSE_BRACE \
    PARAM_CLOSE, BRACE_CLOSE, BRACKET_CLOSE

#define _EX_DATA \
    STRING_LITERAL, WORD, INTEGER

#define _EX_EXPR \
    _EX_DATA, _EX_OPEN_BRACE, _EX_UNARY_OPERATOR

enum Lexicon _PV_default[] = {
  _EX_EXPR,
  0
};
#define _PV_default_len 9

static enum Lexicon _PV_int[] = {
  _EX_BIN_OPERATOR,
  _EX_CLOSE_BRACE,
  _EX_DELIM,
  0
};
#define _PV_int_len 26

static enum Lexicon _PV_word[] = {
  _EX_BIN_OPERATOR,
  DOT,
  _EX_ASN_OPERATOR,
  _EX_CLOSE_BRACE,
  PARAM_OPEN,
  BRACKET_OPEN,
  _EX_DELIM,
  0
};
#define _PV_word_len 35

static enum Lexicon _PV_str[] = {
  _EX_BIN_OPERATOR,
  DOT,
  _EX_CLOSE_BRACE,
  BRACKET_OPEN,
  _EX_DELIM,
  0
};
#define _PV_str_len 28

//TODO probably deserves its own mode
static enum Lexicon _PV_if[] = {
  PARAM_OPEN,
  0
};

#define _PV_if_len 2

enum Lexicon _PV_dot[] = {
  WORD,
  0
};
#define _PV_dot_len 2

enum Lexicon exp_close_param[] = {
  _EX_BIN_OPERATOR,
  _EX_CLOSE_BRACE,
  DOT,
  PARAM_OPEN,
  BRACKET_OPEN,
  _EX_DELIM,
  0
};
#define _PV_close_param_len 29

enum Lexicon _PV_kw[] = {
  IF,
  FUNC_DEF,
  IMPORT,
  0
};
#define _PV_kw_len 4

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

/*
 * Write `_EX_EXPR` into state->expecting 
 */
void init_expect_buffer(struct Previsioner *state)
{   
    memcpy(state->buffer, _PV_default, sizeof(enum Lexicon) * 8);
    state->buffer[9] = 0;
    /* cast removes cc warning */
    state->data.default_mode.ref = (enum Lexicon *)&state->buffer;
    state->mode = EXM_Default; 
}

bool can_addon_keywords(struct Token *ophead)
{
  if(ophead)
    return op_precedence(ophead->type) == 0;
  return true;
}

bool is_expecting_else(struct Expr *expr_head)
{
  //(state->expr_ctr > 0) &&
  return expr_head->type == IfExprT;
}

uint8_t prevision_keywords(enum Lexicon *buf, struct Expr *expr_head) {
  uint8_t offset = _PV_kw_len - 1;

  memcpy(buf, _PV_kw, sizeof(enum Lexicon) * offset);

  if(expr_head->type == IfExprT)
  {
    buf[offset] = ELSE;
    offset += 1;
  }

  buf[offset] = 0;
  offset += 1;
  
  return offset;
}

/* 
 * @param ref NULL terminated  
 */
int8_t prevision_next(struct ExprParserState *state)
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
      state->expecting.mode = EXM_DefSignature;
      break;

    case IMPORT:
      offset = _PV_import_init_len  - 1;
      ref = _PV_import_init;
      state->expecting.mode = EXM_Import; 
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

  if(can_addon_keywords(op_head(state)))
  {
    memcpy(state->expecting.buffer + sizeof(enum Lexicon) * offset, _PV_kw, _PV_kw_len - 1);
    offset += _PV_kw_len - 1;
  }

  if(state->expr_ctr > 0 && is_expecting_else(state->expr_stack[state->expr_ctr - 1]))
  {
    state->expecting.buffer[offset + 1] = ELSE;
    offset += 1; 
  }

  state->expecting.buffer[offset + 1] = 0;

  return 0;
}

/* setup delimiter expectation */
enum Lexicon get_expected_delimiter(struct Group *ghead)
{  
  if(ghead->state & GSTATE_CTX_CODE_GRP)
    return SEMICOLON;
  
  else if(ghead->state & GSTATE_CTX_DATA_GRP)
    return COMMA;
  
  else if (ghead->state & GSTATE_CTX_IDX)
    return COLON;
  
  else if (ghead->state & GSTATE_CTX_MAP_GRP)
  {
    if (ghead->delimiter_cnt % 2 == 0)
      return COMMA;
    else
      return COLON;
  }
  else
    return 0;
}

enum ModeResult {
  _MRMatchFailure = 0,
  _MRContinue = 1,
  _MRComplete = 2,
  _MRError = -1
};


/* 2 = completed
 * 1 = input good, continue going
 * 0 = failure to match token
 * -1 = error
 * */
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
      return (current == WORD && mod) || (current == COMMA && !mod);
  }

  return ret;
}

enum ModeResult mode_import(enum Lexicon current,  struct Previsioner *state)
{
  uint16_t mod = 0;
  enum Lexicon blah[15];
  enum Lexicon *ref;
  uint16_t offset = 0;

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
  if (!contains_tok(current, expecting->data.default_mode.ref))
    return -1;

  /* if current is delimiter, is correct delimiter? */
  else if(is_delimiter(current) && grp_delim != current)
    return -1;
  
  return 0;
}

int8_t is_token_unexpected(struct ExprParserState *state)
{
  struct Token *current = &state->src[*state->_i];
  struct Group *ghead = 0;
  int8_t mode_ret = 0;
  enum Lexicon delim = 0;
  
  if(state->expecting.mode == EXM_Default
     && mode_default(current->type, get_expected_delimiter(ghead), &state->expecting) == -1)
     return -1;
  
  else if(state->expecting.mode == EXM_DefSignature)
  {
    mode_ret = mode_func_def(current->type, &state->expecting);
    
    if(mode_ret == 0)
	    return true;
    
    else if(mode_ret == 1)
	    return false;
    
    else if(mode_ret == 2)
    {
	    state->expecting.mode = EXM_Default;
	    return false;
    }
    else return -1;
  }
  /* expecting buffers contain all delimiters, 
   * so we may check any */
  //else if(!contains_tok(SEMICOLON, state->expecting.exp_ref))
    /* incomplete expression, must get another token */
   // state->panic_flags |= STATE_INCOMPLETE;
  //else 
  //  state->panic_flags &= (uint16_t)STATE_INCOMPLETE;

  /* setup next token's expectation */
  return prevision_next(state);
}
