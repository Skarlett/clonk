#include "clonk.h"
#include "lexer.h"
#include "parser.h"
#include "libtest/CuTest.h"

enum onk_slot_ty
{
  onk_static_slot,
  onk_dynamic_slot,
  onk_inspect_slot
};

/*
 *                              a1
 * [ Dtoken<WORD>, Dtoken<INT>, Dtoken< ? >, 0 ]
 *                               001   Add
 *                               010   Sub
 *                               100   Mul
 * */

struct onk_desc_dyn_token_t {
    enum onk_lexicon_t *arr;
    uint16_t narr;
};


#define FINSP_START 1
#define FINSP_END   1 << 2
#define FINSP_SEQ   1 << 3

struct onk_desc_inspect_token_t
{
    struct onk_token_t token;
    uint8_t ignore_flags;
};

struct onk_desc_token_t {
    enum onk_slot_ty slot_type;

    union {
        enum onk_lexicon_t static_tok;
        struct onk_desc_inspect_token_t inspect;
        struct onk_desc_dyn_token_t dyn_tok;
    } data;
};

#define ONK_TEST_INSP_SZ 64

struct onk_stage_test {
    const char * src_code;
    const char * fp;

    struct onk_token_t *source; // lexed||parsed
    enum onk_lexicon_t *expected;

    uint16_t inspect_arr[ONK_TEST_INSP_SZ];
    uint8_t ninspect;

    uint16_t nsource;
    uint16_t source_sz;

    uint16_t nexpected;
    uint16_t expected_sz;

    uint16_t line;
};


/*
 * formats string, tokenizes, parses,
 *
 * checks output against --
 * */
struct onk_test_mask_t
{
    struct onk_desc_token_t * arr;

    /*number*/
    uint16_t narr;

    /*size*/
    uint16_t sarr;

    uint8_t ignore_whitespace;
    uint8_t ignore_global_scope;
};

void onk_desc_init(
    struct onk_test_mask_t *kit,
    struct onk_desc_token_t *buffer,
    uint16_t buffer_sz
);

/*
 * copies (`enum onk_lexicon_t *tok`) each item into
 * `onk_test_mask_t` as a static option.
*/
int8_t onk_desc_add_static_slot(
    struct onk_test_mask_t * mold,
    enum onk_lexicon_t tok[],
    uint16_t nitems
);

int8_t onk_desc_add_inspect_slot(
    struct onk_test_mask_t *kit,
    struct onk_desc_inspect_token_t *inspect
);

int onk_desc_add_dynamic_slot(
    struct onk_test_mask_t * kit,
    enum onk_lexicon_t *answers,
    uint16_t nanswers);

int8_t onk_desc_add_static_repeating_slot(
    struct onk_test_mask_t *kit,
    enum onk_lexicon_t tok,
    uint16_t ntimes
);

int8_t onk_desc_add_repeating_slot(
    struct onk_test_mask_t *kit,
    struct onk_desc_token_t *tok,
    uint16_t ntimes
);

int8_t onk_assert_match(
    CuTest *tc,
    struct onk_test_mask_t *kit,
    struct onk_token_t input[],
    uint16_t ninput,
    uint16_t iter,

    const char * src_code,
    char * filepath,
    uint16_t line
);

int8_t onk_assert_tokenize(
    CuTest *tc,
    struct onk_test_mask_t *kit,
    struct onk_lexer_input_t *lexer_input,
    struct onk_lexer_output_t *lexer_output,
    uint16_t iter,
    char * fp,
    uint16_t line
);

int8_t onk_assert_postfix(
    CuTest *tc,
    struct onk_test_mask_t *kit,
    struct onk_parser_input_t *input,
    struct onk_parser_output_t *output,
    uint16_t i,
    char * fp,
    uint16_t line
);

int8_t onk_assert_parse_stage(
    CuTest *tc,
    struct onk_test_mask_t *lexer,
    struct onk_test_mask_t *parser,
    uint16_t fmt_i,
    char *fp,
    uint16_t line
);

#define OnkAssertTokenMask(tc, kit, toks, ntoks, i)     \
    onk_assert_match((tc), (kit), (toks), ntoks, i, __FILE__, __LINE__);

#define OnkAssertLexerStage(tc, kit, input, output, i)                  \
    onk_assert_tokenize((tc), (kit), (input), (output), (i), __FILE__, __LINE__)

#define OnkAssertPostFixStage(tc, kit, input, output, i)                \
    onk_assert_postfix((tc), (kit), (input), (output), (i), __FILE__, __LINE__))

#define OnkAssertParseStage(tc, kit, input, i)  \
    onk_assert_parse_stage((tc), (kit), (input), (i), __FILE__, __LINE__)
