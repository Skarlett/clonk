#include "clonk.h"
#include "libtest/CuTest.h"
#include "libtest/masking.h"
#include "libtest/assert.h"

#define MOCK_SRC_LEN 9
static const char * MOCK_SRC = "word word";

static enum onk_lexicon_t SUCCEED_MOCK[] =      \
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN};

static const enum onk_lexicon_t FAIL_MOCK[] =   \
        {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN};

void mk_toks(struct onk_token_t token[], const enum onk_lexicon_t ty[]) {
  for (uint16_t i = 0 ;; i++)
    if (ty[i] == 0)
      break;
    else
      token[i].type = ty[i];
}

void __test__check_tokens(CuTest* tc) {
     struct onk_token_t toks[8];

     const static enum onk_lexicon_t check_list[][16] = {
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_DIV_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_ADD_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_MUL_TOKEN, ONK_SUB_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_ADD_TOKEN, 0},
    };

    for (unsigned char i=0; 6 > i; i++){
        mk_toks(toks, check_list[i]);
        onk_assert_tokens(tc, toks, check_list[i], "n/a", "file", 0);
    }
}

void __test__mock_tokens(CuTest *tc)
{
    struct onk_token_t tokens[2];
    enum onk_lexicon_t arr[2] = {ONK_WORD_TOKEN, ONK_ADD_TOKEN};

    create_mock_tokens(tokens, 2, arr);
    for (unsigned char i=0; 2 > i; i++)
        CuAssertTrue(tc, tokens[i].type == arr[i]);
}

void __test__assert_tokens(CuTest *tc)
{
    struct onk_test_mask_t kit;
    struct CuTest dummy;
    struct onk_token_t tokens[2];
    int8_t ret;

    create_mock_tokens(tokens, 2, SUCCEED_MOCK);

    ret = onk_assert_tokens(
        tc, tokens, SUCCEED_MOCK, "failed", "NA", 0);

    CuAssert(tc, "failed token equality check", ret == 0);
    CuAssertTrue(tc, dummy.failed == 0);

    ret = onk_assert_tokens(
        tc, tokens, FAIL_MOCK, "failed", "NA", 0);

    CuAssert(tc, "failed token equality check", ret != 0);
    CuAssertTrue(tc, dummy.failed);
}

void __test__assert_match_tokens(CuTest *tc)
{
    struct onk_test_mask_t kit;
    struct CuTest dummy;
    struct onk_token_t tokens[2];
    int8_t ret;

    create_mock_tokens(tokens, 2, SUCCEED_MOCK);

    onk_desc_add_static_slot(&kit, SUCCEED_MOCK, 2);

    ret = onk_assert_match(
        &dummy, &kit, tokens, 2, 0, "failed", "NA", 0);

    CuAssert(tc, "failed static-match check", ret == 0);
    CuAssertTrue(tc, dummy.failed == 0);

    ret = onk_assert_match(
        &dummy, &kit, tokens, 2, 0, "failed", "NA", 0);

    CuAssert(tc, "failed token equality check", ret != 0);
    CuAssertTrue(tc, dummy.failed);
}


void __test__assert_tokenize_stage(CuTest *tc, struct onk_test_state_t *buffers)
{
    struct CuTest dummy;
    struct onk_test_mask_t kit;
    struct onk_lexer_input_t lin;
    struct onk_lexer_output_t lout;
    int8_t ret;

    onk_desc_add_static_slot(&kit, SUCCEED_MOCK, 2);

    lin.src_code = MOCK_SRC;
    lin.tokens = buffers->src_tokens;

    ret = onk_assert_tokenize(&dummy, &kit, &lin, &lout, 0, "NA", 0);

    CuAssert(tc, "failed tokenize stage", ret == 0);
    CuAssertTrue(tc, lout.tokens.len == 2); // eof?
}

void __test__assert_postfix_stage(CuTest *tc, struct onk_test_state_t *buffers)
{
    struct CuTest dummy;
    struct onk_test_mask_t kit;
    struct onk_parser_input_t pin;
    struct onk_parser_output_t pout;
    int8_t ret;

    onk_desc_add_static_slot(&kit, SUCCEED_MOCK, 2);

    pin.src_code = MOCK_SRC;
    pin.src_code_sz = MOCK_SRC_LEN;

    pin.tokens = buffers->src_tokens;
    pin.add_glob_scope = false;

    ret = onk_assert_postfix(&dummy, &kit, &pin, &pout, 0, "NA", 0);
    CuAssert(tc, "failed parsing stage", ret == 0);
}


// WARNINGS OKAY
void __test__assert_parse_stage(CuTest *tc)
{
    CuTest dummy;
    const char * src = "word + word";
    static const enum onk_lexicon_t lexed[] = {ONK_WORD_TOKEN, ONK_ADD_TOKEN, ONK_WORD_TOKEN};
    static const enum onk_lexicon_t parsed[] = {ONK_WORD_TOKEN,  ONK_WORD_TOKEN, ONK_ADD_TOKEN};
    struct onk_test_mask_t lexer, parser;
    int ret;

    onk_desc_add_static_slot(&lexer, lexed, 3);
    onk_desc_add_static_slot(&parser, parsed, 3);

    ret = onk_assert_parse_stage(&dummy, &lexer, &parser, 0, "NA", 0);
    CuAssert(tc, "failed parsing stage", ret == 0);

    ret = onk_assert_parse_stage(&dummy, &parser, &lexer, 0, "NA", 0);
    CuAssert(tc, "failed parsing stage", ret == -1);
}


CuSuite* OnkAssertTests(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__mock_tokens);
    SUITE_ADD_TEST(suite, __test__assert_match_tokens);
    SUITE_ADD_TEST(suite, __test__assert_tokens);

    SUITE_ADD_STATE_TEST(suite, __test__assert_tokenize_stage);
    SUITE_ADD_STATE_TEST(suite, __test__assert_postfix_stage);
    SUITE_ADD_TEST(suite, __test__assert_parse_stage);

    return suite;
}
