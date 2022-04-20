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

void test_mock_tokens(CuTest *tc)
{
    struct onk_token_t tokens[2];
    enum onk_lexicon_t arr[2] = {ONK_WORD_TOKEN, ONK_ADD_TOKEN};

    create_mock_tokens(tokens, 2, arr);
    for (uint8_t i=0; 2 > i; i++)
        CuAssertTrue(tc, tokens[i].type == arr[i]);
}

void test_assert_tokens(CuTest *tc)
{
    struct onk_test_mask_t kit;
    struct CuTest dummy;
    struct onk_token_t tokens[2];
    int8_t ret;

    create_mock_tokens(tokens, 2, SUCCEED_MOCK);
    ret = onk_assert_tokens(
        tc, tokens, SUCCEED_MOCK, "N/A", "failed", "NA", 0);

    CuAssert(tc, "failed token equality check", ret == 0);
    CuAssertTrue(tc, dummy.failed == 0);

    ret = onk_assert_tokens(
        tc, tokens, FAIL_MOCK, "N/A", "failed", "NA", 0);

    CuAssert(tc, "failed token equality check", ret != 0);
    CuAssertTrue(tc, dummy.failed);
}

void test_assert_match_tokens(CuTest *tc)
{
    struct onk_test_mask_t kit;
    struct CuTest dummy;
    struct onk_token_t tokens[2];
    int8_t ret;

    create_mock_tokens(tokens, 2, SUCCEED_MOCK);
    onk_desc_add_static_slot(&kit, SUCCEED_MOCK, 2);

    ret = onk_assert_match(
        &dummy, &kit, tokens, 2, 0, "N/A", "failed", 0);

    CuAssert(tc, "failed static-match check", ret == 0);
    CuAssertTrue(tc, dummy.failed == 0);

    ret = onk_assert_match(
        &dummy, &kit, tokens, 2, 0, "N/A", "failed", 0);

    CuAssert(tc, "failed token equality check", ret != 0);
    CuAssertTrue(tc, dummy.failed);
}


void test_assert_tokenize_stage(CuTest *tc)
{
    struct CuTest dummy;
    struct onk_test_mask_t kit;
    struct onk_lexer_input_t lin;
    struct onk_lexer_output_t lout;
    int8_t ret;

    onk_desc_add_static_slot(&kit, SUCCEED_MOCK, 2);

    lin.src_code = MOCK_SRC;
    lin.tokens = tc->buffers->src_tokens;

    ret = onk_assert_tokenize(&dummy, &kit, &lin, &lout, 0, "NA", 0);

    CuAssert(tc, "failed tokenize stage", ret == 0);
    CuAssertTrue(tc, lout.tokens.len == 2); // eof?
}

void test_assert_postfix_stage(CuTest *tc)
{
    struct CuTest dummy;
    struct onk_test_mask_t kit;
    struct onk_parser_input_t pin;
    struct onk_parser_output_t pout;
    int8_t ret;

    onk_desc_add_static_slot(&kit, SUCCEED_MOCK, 2);

    pin.src_code = MOCK_SRC;
    pin.src_code_sz = MOCK_SRC_LEN;

    pin.tokens = tc->buffers->src_tokens;
    pin.add_glob_scope = false;

    ret = onk_assert_postfix(&dummy, &kit, &pin, &pout, 0, "NA", 0);
    CuAssert(tc, "failed parsing stage", ret == 0);
}


void test_assert_parse_stage(CuTest *tc)
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
