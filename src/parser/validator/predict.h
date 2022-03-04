
#ifndef __ONK_PRIVATE_VALIDATOR__
#define __ONK_PRIVATE_VALIDATOR__

#include "clonk.h"
#include "lexer.h"

/* because it can't be applied to integers */
/* unlike the other operators which can be applied */
/* to groupings, words & integers */

#define _EX_UNIT ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, \
    ONK_NULL_TOKEN, ONK_TRUE_TOKEN, ONK_FALSE_TOKEN,  \
    ONK_STRING_LITERAL_TOKEN
#define UNIT_LEN 6
const enum onk_lexicon_t UNIT[UNIT_LEN] = {_EX_UNIT};

#define _EX_BIN_OPERATOR                 \
  ONK_ADD_TOKEN, ONK_MUL_TOKEN, ONK_SUB_TOKEN,      \
    ONK_DIV_TOKEN, ONK_POW_TOKEN, ONK_MOD_TOKEN,                        \
    ONK_PIPE_TOKEN, ONK_AMPER_TOKEN, ONK_OR_TOKEN, ONK_AND_TOKEN,                \
    ONK_LT_TOKEN, ONK_LT_EQL_TOKEN, ONK_SHL_TOKEN,                       \
    ONK_GT_TOKEN, ONK_GT_EQL_TOKEN, ONK_SHR_TOKEN

#define BINOP_LEN 16
const enum onk_lexicon_t BINOP[BINOP_LEN] = {_EX_BIN_OPERATOR};

#define _EX_ASN_OPERATOR                                    \
    ONK_EQUAL_TOKEN, ONK_PLUSEQ_TOKEN, ONK_MINUS_EQL_TOKEN, \
    ONK_BIT_AND_EQL, ONK_BIT_OR_EQL, ONK_BIT_NOT_EQL
#define ASNOP_LEN 6
const enum onk_lexicon_t ASNOP[ASNOP_LEN] = {_EX_ASN_OPERATOR};

#define _EX_UNARY_OPERATOR ONK_TILDE_TOKEN, ONK_NOT_TOKEN
#define UNIOP_LEN 2
const enum onk_lexicon_t UNIOP[UNIOP_LEN] = {_EX_UNARY_OPERATOR};

#define _EX_OPEN_BRACE ONK_PARAM_OPEN_TOKEN, ONK_BRACE_OPEN_TOKEN,  \
    ONK_BRACKET_OPEN_TOKEN, ONK_HASHMAP_LITERAL_START_TOKEN
#define BRACE_OPEN_LEN 4
const enum onk_lexicon_t OPEN_BRACE[BRACE_OPEN_LEN] = {_EX_OPEN_BRACE};

#define _EX_CLOSE_BRACE ONK_PARAM_CLOSE_TOKEN, ONK_BRACE_CLOSE_TOKEN, ONK_BRACKET_CLOSE_TOKEN
#define BRACE_CLOSE_LEN 3
const enum onk_lexicon_t CLOSE_BRACE[BRACE_CLOSE_LEN] = {_EX_CLOSE_BRACE};

#define _EX_EXPR _EX_OPEN_BRACE, _EX_UNARY_OPERATOR, _EX_UNIT
#define EXPR_LEN BRACE_OPEN_LEN + UNIOP_LEN + UNIT_LEN
const enum onk_lexicon_t EXPR[EXPR_LEN] = {_EX_EXPR};

#define _EX_KWORD_BLOCK                             \
      ONK_IF_TOKEN, ONK_DEF_TOKEN, ONK_FROM_TOKEN,  \
      ONK_IMPORT_TOKEN, ONK_STRUCT_TOKEN,           \
      ONK_FOR_TOKEN, ONK_WHILE_TOKEN, ONK_RETURN_TOKEN
#define _EX_KWORD_BLOCK_LEN 8
const enum onk_lexicon_t BLOCK_KWORD[_EX_KWORD_BLOCK_LEN];


#define _NEXT_UNIT_GENERIC _EX_BIN_OPERATOR /*terminator*/
#define _NEXT_UNIT_GENERIC_LEN BINOP_LEN

#define _NEXT_WORD _NEXT_UNIT_GENERIC, ONK_DOT_TOKEN, ONK_BRACE_OPEN_TOKEN, \
    ONK_PARAM_OPEN_TOKEN, ONK_BRACKET_OPEN_TOKEN, _EX_ASN_OPERATOR
#define NEXT_WORD_LEN _NEXT_UNIT_GENERIC_LEN + 4 + ASNOP_LEN
const enum onk_lexicon_t NEXT_WORD[NEXT_WORD_LEN] = {_NEXT_WORD};

#define NEXT_STRING _NEXT_UNIT_GENERIC, DOT, ONK_OPEN_BRACKET_TOKEN
#define NEXT_STRING_LEN _NEXT_UNIT_GENERIC_LEN + 2


#define NEXT_OPERATOR _EX_EXPR
#define NEXT_OPERATOR_LEN EXPR_LEN

#define NEXT_OPEN_BRACE _EX_EXPR
#define NEXT_OPEN_BRACE_LEN EXPR_LEN

#define NEXT_DELIM _EX_EXPR
#define NEXT_DELIM_LEN EXPR_LEN

#define _NEXT_CLOSE_BRACE ONK_DOT_TOKEN, _EX_BIN_OPERATOR, \
    ONK_BRACKET_OPEN_TOKEN, ONK_PARAM_OPEN_TOKEN /*delim*/

const enum onk_lexicon_t NEXT_CLOSE_BRACE[] = {_NEXT_CLOSE_BRACE};


#define NEXT_DOT ONK_WORD_TOKEN


//#define NEXT_IF ONK_OPEN_PARAM_TOKEN


enum validator_no {

  expr_mode_t,

  /*def foo(x=y + s, z=w)*/
  /*struct Foo{x=y, z, d=a}*/
  parameter_mode_t,

  /*
   * Must follow sequence of tokens exactly
  */
  sequence_mode,


  /* must follow a block*/
  /* if(..) **{ }** */
  /* def foo(..) **{ }** */
  /* impl foo **{ }** */
  /* while(..) **{ }** */
  /* */
  follow_block



};

struct validator_frame_t {
  enum validator_no mode;
  bool allow_delim;
  bool allow_open_brace;



};

#define _ONK_VALIDATOR_SZ 24
#define _ONK_VALIDATOR_REF_SZ 8
struct validator_t {
  enum onk_lexicon_t * ref_buffer[_ONK_VALIDATOR_REF_SZ];
  enum onk_lexicon_t insert[_ONK_VALIDATOR_SZ];
  struct validator_frame_t stack[ONK_STACK_SZ];

  uint8_t nref;
  uint8_t ninsert;
  uint16_t nstack;
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
