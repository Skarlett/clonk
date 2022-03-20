
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
#define EXPR_SZ sizeof(enum onk_lexicon_t) * EXPR_LEN
const enum onk_lexicon_t EXPR[EXPR_LEN] = {_EX_EXPR};


#define _EX_KWORD_BLOCK                             \
      ONK_IF_TOKEN, ONK_DEF_TOKEN, ONK_FROM_TOKEN,  \
      ONK_IMPORT_TOKEN, ONK_STRUCT_TOKEN,           \
      ONK_FOR_TOKEN, ONK_WHILE_TOKEN, ONK_RETURN_TOKEN


#define KWORD_BLOCK_LEN 8
#define KWORD_BLOCK_SZ sizeof(enum onk_lexicon_t) * KWORD_BLOCK_LEN
const enum onk_lexicon_t KWORD_BLOCK[KWORD_BLOCK_LEN];



#define _NEXT_CLOSE_BRACE ONK_DOT_TOKEN, _EX_BIN_OPERATOR, \
    ONK_BRACKET_OPEN_TOKEN, ONK_PARAM_OPEN_TOKEN /*delim*/
#define NEXT_CLOSE_BRACE_LEN BINOP_LEN + 3
const enum onk_lexicon_t NEXT_CLOSE_BRACE[] = { _NEXT_CLOSE_BRACE};


#define _ONK_VALIDATOR_SZ 16
#define _ONK_VALIDATOR_REF_SZ 4

struct validator_t {
  enum onk_lexicon_t * slices[_ONK_VALIDATOR_REF_SZ];
  enum onk_lexicon_t buffer[_ONK_VALIDATOR_SZ];

 uint16_t islices[_ONK_VALIDATOR_REF_SZ];
  uint16_t nslices;
  uint16_t nbuffer;
};

void init_expect_buffer(enum onk_lexicon_t *arr);
bool is_token_unexpected(struct onk_parser_state_t*state);
#endif
