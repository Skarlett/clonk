#include <stdint.h>
#include <string.h>
#include <assert.h>
#include "expect.h"
#include "../lexer/lexer.h"
#include "../lexer/helpers.h"
#include "expr.h"

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

/*
    check flag, and if present, unset it.
*/
FLAG_T check_flag(FLAG_T set, FLAG_T flag){
    return set & flag;
}
void set_flag(FLAG_T *set, FLAG_T flag){
    *set = *set | flag;
}

void unset_flag(FLAG_T *set, FLAG_T flag){
    *set = *set & ~flag;
}

enum Lexicon exp_default[] = {
  _EX_EXPR,
  0
};

enum Lexicon exp_int[] = {
  _EX_BIN_OPERATOR,
  _EX_CLOSE_BRACE,
  _EX_DELIM,
  0
};

enum Lexicon exp_word[] = {
  _EX_BIN_OPERATOR,
  DOT,
  _EX_ASN_OPERATOR,
  _EX_CLOSE_BRACE,
  PARAM_OPEN,
  BRACKET_OPEN,
  _EX_DELIM,
  0
};

enum Lexicon exp_str[] = {
  _EX_BIN_OPERATOR,
  DOT,
  _EX_CLOSE_BRACE,
  BRACKET_OPEN,
  _EX_DELIM,
  0
};

enum Lexicon exp_if[] = {
  PARAM_OPEN,
  0
};

enum Lexicon exp_dot[] = {
  WORD,
  0
};

enum Lexicon exp_close_param[] = {
  _EX_BIN_OPERATOR,
  _EX_CLOSE_BRACE,
  DOT,
  PARAM_OPEN,
  BRACKET_OPEN,
  _EX_DELIM,
  0
};

/*
 * Write `_EX_EXPR` into state->expecting 
 */
void init_expect_buffer(struct ExprParserState *state)
{   
    memcpy(state->expecting, exp_default, sizeof(enum Lexicon) * 8);
    state->expecting[9] = 0;
    state->expecting_ref = state->expecting;
}

/* 
 * @param state->expecting_ref NULL terminated  
 */
int8_t next_token(struct ExprParserState *state)
{
  enum Lexicon current = state->src[*state->_i].type;
  uint16_t total_size = 0;

  if (current == WORD)
    state->expecting_ref = exp_word;
  
  else if (current == INTEGER)
    state->expecting_ref = exp_int;
  
  else if (current == STRING_LITERAL)
    state->expecting_ref = exp_word;
  
  else if (current == DOT)
    state->expecting_ref = exp_dot;
  
  else if (current == IF)
   state->expecting_ref = exp_if;
  
  else if(current == ELSE)
    state->expecting_ref = exp_default;

  else if(current == RETURN)
    state->expecting_ref = exp_default;
  
  else if (is_operator(current))
    state->expecting_ref = exp_default;

  else if (is_delimiter(current))
    state->expecting_ref = exp_default;

  /* any open brace */
  else if (is_open_brace(current))
  { 
    /* add opposite brace type to expectation */
    state->expecting[8] = invert_brace_tok_ty(current);
    state->expecting_ref = state->expecting;
  }

  /* any open brace */
  else if (is_close_brace(current)) 
    state->expecting_ref = exp_close_param;
  
  else
    return -1;
	
  return 0;
}

enum Lexicon get_expected_delimiter(struct Group *ghead) {
  /* setup delimiter expectation */
  if(check_flag(GSTATE_CTX_CODE_GRP, ghead->state))
    return SEMICOLON;
  
  else if(check_flag(GSTATE_CTX_DATA_GRP, ghead->state))
    return COMMA;
  
  else if (check_flag(GSTATE_CTX_IDX, ghead->state))
    return COLON;
  
  else if (check_flag(GSTATE_CTX_MAP_GRP, ghead->state)) {
    if (ghead->delimiter_cnt % 2 == 0)
      return COMMA;
    else
      return COLON;
    // 2:3, 3:4
  }

  else
    return 0;
}

int8_t is_token_unexpected(struct ExprParserState *state) {
  struct Token *current = &state->src[*state->_i];
  struct Group *ghead = 0;
  enum Lexicon delim = 0;
 
  if (state->set_ctr > 0)
    ghead = &state->set_stack[state->set_ctr - 1];
  else
    ghead = &state->set_stack[0];

  delim = get_expected_delimiter(ghead);

  /* check previous expecting buffer */
  if (!contains_tok(current->type, state->expecting_ref))
    return 1;

  /* if current is delimiter, is correct delimiter? */
  else if(is_delimiter(current->type) && get_expected_delimiter(ghead) != current->type)
    return 1;
  
  /* expecting buffers contain all delimiters, 
   * so we may check any */
  else if(!contains_tok(SEMICOLON, state->expecting_ref))
    /* incomplete expression, must get another token */
    set_flag(&state->panic_flags, STATE_INCOMPLETE);
  else 
    unset_flag(&state->panic_flags, STATE_INCOMPLETE);

  /* setup next token's expectation */
  return next_token(state);
}
