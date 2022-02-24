
#ifndef __ONK_PRIVATE_VALIDATOR__
#define __ONK_PRIVATE_VALIDATOR__

#include "clonk.h"
#include "lexer.h"

/* because it can't be applied to integers */
/* unlike the other operators which can be applied */
/* to groupings, words & integers */

#define _EX_BIN_OPERATOR                 \
    ONK_ADD_TOKEN, ONK_MUL_TOKEN, ONK_SUB_TOKEN, ONK_DIV_TOKEN, ONK_POW_TOKEN, ONK_MOD_TOKEN,        \
    ONK_PIPE_TOKEN, ONK_AMPER_TOKEN, ONK_OR_TOKEN, ONK_AND_TOKEN,                \
    ONK_LT_TOKEN, ONK_LT_EQL_TOKEN, ONK_SHL_TOKEN,                       \
    ONK_GT_TOKEN, ONK_GT_EQL_TOKEN, ONK_SHR_TOKEN                        \
    //ONK_MINUS_EQL_TOKEN, ONK_PLUSEQ_TOKEN
    // ONK_TILDE_TOKEN, ONK_NOT_TOKEN
#define _EX_BIN_OPERATOR_LEN 16


#define _EX_UNARY_OPERATOR ONK_TILDE_TOKEN, ONK_NOT_TOKEN
#define _EX_UNARY_OPERATOR_LEN 2

#define _EX_DELIM ONK_COMMA_TOKEN, ONK_COLON_TOKEN, ONK_SEMICOLON_TOKEN
#define _EX_DELIM_LEN 3

#define _EX_ASN_OPERATOR \
    ONK_EQUAL_TOKEN, ONK_PLUSEQ_TOKEN, ONK_MINUS_EQL_TOKEN, \
    ONK_BIT_AND_EQL, ONK_BIT_OR_EQL, ONK_BIT_NOT_EQL
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
  _EX_OPEN_BRACE,

#define _EX_EXPR_LEN                            \
  _EX_EXPR_LIMITED_LEN                           \
  + _EX_BRACE_LEN


/*
 * ONK_ELSE_TOKEN is not included,
 * because it needs special checks
*/
#define _EX_KEYONK_WORD_TOKEN                             \
    ONK_IF_TOKEN, ONK_DEF_TOKEN, ONK_FROM_TOKEN,                         \
    ONK_IMPORT_TOKEN, ONK_STRUCT_TOKEN, ONK_FOR_TOKEN, ONK_WHILE_TOKEN, ONK_RETURN_TOKEN

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

enum PrevisionerModeT {
  /* give list of next possible
   * tokens based on the input */
  PV_Default,

  /* follow a sequence of
   * tokens until completed */
  PV_DefSignature,
  PV_Import
};




enum ValidatorMode {

  /*
   * Must follow sequence of tokens exactly
  */
  strict_mode_tight,

  /* white list of tokens, but changes mode
   * once terminator is found */
  strict_mode_loose,


  mode_default,
};

struct ValidatorFrame {
  enum ValidatorMode mode;
  bool allow_delim;
  bool allow_open_brace;
};

struct ValidatorState {
  enum onk_lexicon_t * buffer;
  struct ValidatorFrame * stack;
};


bool kw_follows_open_param(enum onk_lexicon_t tok);

bool follows_word(enum onk_lexicon_t tok);

bool can_use_else(enum onk_lexicon_t output_head);

/*
 * Can use Keywords if operator stack is flushed
 * and the top frame is a codeblock
*/
bool can_use_keywords(struct Parser *state);


/* Predicts the next possible tokens
 * from the current token.
 * Used to check for unexpected tokens.
 * functionality is
*/
void init_expect_buffer(struct Previsioner *state);
#endif
