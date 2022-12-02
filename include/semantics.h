#ifndef __ONK_PRIVATE_VALIDATOR__
#define __ONK_PRIVATE_VALIDATOR__

#include "clonk.h"
#include "lexer.h"
#include "parser.h"

/* because it can't be applied to integers */
/* unlike the other operators which can be applied */
/* to groupings, words & integers */

#define UNIT_LEN 6
#define _EX_UNIT ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, \
    ONK_NULL_TOKEN, ONK_TRUE_TOKEN, ONK_FALSE_TOKEN,  \
    ONK_STRING_LITERAL_TOKEN

#define BINOP_LEN 16
#define _EX_BIN_OPERATOR                                                \
    ONK_ADD_TOKEN, ONK_MUL_TOKEN, ONK_SUB_TOKEN,                        \
    ONK_DIV_TOKEN, ONK_POW_TOKEN, ONK_MOD_TOKEN,                        \
    ONK_PIPE_TOKEN, ONK_AMPER_TOKEN, ONK_OR_TOKEN,                      \
    ONK_AND_TOKEN, ONK_LT_TOKEN, ONK_LT_EQL_TOKEN,                      \
    ONK_SHL_TOKEN,ONK_GT_TOKEN, ONK_GT_EQL_TOKEN,                       \
    ONK_SHR_TOKEN

#define ASNOP_LEN 6
#define _EX_ASN_OPERATOR                                    \
    ONK_EQUAL_TOKEN, ONK_PLUSEQ_TOKEN, ONK_MINUS_EQL_TOKEN, \
    ONK_BIT_AND_EQL, ONK_BIT_OR_EQL, ONK_BIT_NOT_EQL

#define DELIM_LEN 3
#define _EX_DELIM                                           \
    ONK_COLON_TOKEN, ONK_SEMICOLON_TOKEN, ONK_COMMA_TOKEN

#define _EX_UNARY_OPERATOR ONK_TILDE_TOKEN, ONK_NOT_TOKEN
#define UNIOP_LEN 2

#define BRACE_OPEN_LEN 4
#define _EX_OPEN_BRACE ONK_PARAM_OPEN_TOKEN, ONK_BRACE_OPEN_TOKEN,  \
    ONK_BRACKET_OPEN_TOKEN, ONK_HASHMAP_LITERAL_START_TOKEN

#define BRACE_CLOSE_LEN 3
#define _EX_CLOSE_BRACE ONK_PARAM_CLOSE_TOKEN, ONK_BRACE_CLOSE_TOKEN, ONK_BRACKET_CLOSE_TOKEN

#define _EX_EXPR ONK_PARAM_OPEN_TOKEN, ONK_BRACKET_OPEN_TOKEN, \
        ONK_HASHMAP_LITERAL_START_TOKEN, _EX_UNARY_OPERATOR, _EX_UNIT

#define EXPR_LEN (3 + UNIOP_LEN + UNIT_LEN)
#define EXPR_SZ (sizeof(enum onk_lexicon_t) * EXPR_LEN)

#define _EX_KWORD_BLOCK                             \
      ONK_IF_TOKEN, ONK_DEF_TOKEN, ONK_FROM_TOKEN,  \
      ONK_IMPORT_TOKEN, ONK_STRUCT_TOKEN,           \
      ONK_FOR_TOKEN, ONK_WHILE_TOKEN, ONK_RETURN_TOKEN

#define KWORD_BLOCK_LEN 8
#define KWORD_BLOCK_SZ (sizeof(enum onk_lexicon_t) * KWORD_BLOCK_LEN)

#define _ONK_VALIDATOR_SZ 16
struct validator_frame_t {
    enum onk_lexicon_t ** slices;
    uint16_t * islices;
    uint16_t nslices;
    bool set_delim;
    bool set_brace;
};

void init_validator_frame(
    struct validator_frame_t *frame,
    struct onk_parser_state_t *state
);

void onk_semenatic_init(
    struct onk_parser_state_t *state
);

bool onk_semantic_check(
  struct onk_parser_state_t *state,
  enum onk_lexicon_t current
);

uint16_t onk_semantic_compile(
  struct validator_frame_t *validator,
  struct onk_parser_state_t *state
);

int8_t onk_semantic_build_frame(
  struct onk_parser_state_t *state
);

void vframe_add_slice(
  struct validator_frame_t *f,
  enum onk_lexicon_t *slice,
  uint16_t len
);

enum onk_lexicon_t vframe_add_delimiter(
  struct onk_parser_state_t * state
);

#endif
