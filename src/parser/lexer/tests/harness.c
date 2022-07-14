#include <stdio.h>
#include <string.h>

#include "onkstd/merge_sort.h"
#include "libtest/CuTest.h"
#include "libtest/assert.h"
#include "lexer.h"

#define MAX_HARNESS 64

int8_t _onk_range_harness(
    enum onk_lexicon_t *answers,
    handler handler,
    uint16_t *failed_on,
    enum onk_lexicon_t *generated_answers,
    uint16_t *ngenerated,
    uint16_t generated_sz)
{
    enum onk_lexicon_t tok;
    uint8_t answers_ctr = 0, idx=0;

    if(answers == 0
       || failed_on == 0
       || generated_answers == 0
       || ngenerated == 0)
       return -5;

    /* find null delimiter */
    for (answers_ctr=0; MAX_HARNESS > answers_ctr; answers_ctr++)
        if(answers[answers_ctr] == 0)
            break;

    if(answers_ctr == 0)
        return -5;

    /* generate all tokens  */
    for(tok = __ONK_TOKEN_START; __ONK_TOKEN_END > tok; tok++)
    {
        if(handler(tok))
        {
            generated_answers[idx] = tok;
            idx += 1;
            if (idx >= generated_sz) return -2;
        }
    }

    /* no tokens generated from handler? */
    if(idx == 0)
        return -4;

    *ngenerated = idx + 1;

    if (answers_ctr != idx)
        return -3;

    /* only answers need to be sorted,
     * `generated_answers` will be sorted already */
    onk_merge_sort_u16((uint16_t *)answers, 0, answers_ctr);

    for(uint16_t i=0; answers_ctr > i; i++)
    {
        if(generated_answers[i] != answers[i])
        {
            *failed_on = i;
            return -1;
        }
    }

    return 0;
}


void onk_range_harness(
    CuTest *tc,
    char * msg,
    enum onk_lexicon_t *answers,
    handler handler,
    const char * handler_name,
    const char * fp,
    uint16_t line
)
{
    uint16_t failed_on = 0;
    char msg_buf[512];
    char header[128];
    enum onk_lexicon_t generated[MAX_HARNESS];
    uint16_t ngen;
    int8_t ret;

    ret = _onk_range_harness(
        answers, handler, &failed_on, generated, &ngen, MAX_HARNESS
    );

    snprintf(
        header,
        128,
        "%s\n%s:%u [%s][%u] ",
        msg, fp, line, handler_name, failed_on
    );

    switch (ret)
    {
        case 0:
            return;

        case -1:
            snprintf(
                msg_buf,
                512,
                "%s expected <%s>, got <%s> on idx: %u\n",
                header,
                onk_ptoken(generated[failed_on]),
                onk_ptoken(answers[failed_on]),
                failed_on
            );
            CuFail(tc, msg_buf);
            return;

        case -2:
            snprintf(
                msg_buf,
                512,
                "%s generated tokens overflowed\n",
                header
            );
            CuFail(tc, msg_buf);
            break;

        case -3:
            snprintf(
                msg_buf,
                512,
                "%s length-equality failed\n",
                header
            );
            CuFail(tc, msg_buf);
            break;

        case -4:
            snprintf(
                msg_buf,
                512,
                "%s\nno tokens generated from handler\n",
                header
            );
            CuFail(tc, msg_buf);
            break;

        case -5:
            snprintf(
                msg_buf,
                512,
                "%s\nharness called with null pointer, or empty answers\n",
                header
            );
            CuFail(tc, msg_buf);
            break;


        default:
            CuFail(tc, "unreachable code triggered");
            return;
    }

}


/******************
* test foundation *
******************/

void __test__illegal_tokens_length(CuTest *tc) {
    static const char *msg = "the amount of illegal tokens "  \
            "does not match ILLEGAL_TOKEN_LEN";

    for(uint8_t i=0; UINT8_MAX > i; i++)
    {
        if(ILLEGAL_TOKENS[i] == 0)
        {
          if(ILLEGAL_TOKENS_LEN == i)
              return;

          else CuFail(tc, msg);
        }
    }

    CuFail(tc, msg);
}

/****************
* test overflow *
****************/

bool test_handle_overflow(enum onk_lexicon_t _)
{ return true; }

void __test__lex_harness_overflow(CuTest *tc)
{
    uint16_t failed_on = 0, ngenerated = 0;
    int8_t ret;

    enum onk_lexicon_t generated[2];
    enum onk_lexicon_t wrong[] = {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, 0};

    ret = _onk_range_harness(
        wrong, test_handle_overflow, &failed_on,
        generated, &ngenerated, 2);

    CuAssertTrue(tc, ret == -4);
}

/***************
* Null answers *
***************/

void __test__lex_harness_empty_arr(CuTest *tc)
{
    uint16_t failed_on = 0, ngenerated = 0;
    int8_t ret;
    enum onk_lexicon_t generated[MAX_HARNESS];
    enum onk_lexicon_t answers[] = { 0 };

    ret = _onk_range_harness(answers, test_handle_overflow, &failed_on,
                       generated, &ngenerated, MAX_HARNESS);

    CuAssertTrue(tc, ret == -5);
}

void __test__lex_harness_null_ptr(CuTest *tc)
{
    uint16_t failed_on = 0, ngenerated = 0;
    int8_t ret;
    enum onk_lexicon_t generated[MAX_HARNESS];

    ret = _onk_range_harness(0, test_handle_overflow, &failed_on,
                       generated, &ngenerated, MAX_HARNESS);

    CuAssertTrue(tc, ret == -5);
}

/****************
* test bad hook *
****************/
bool bad_hook(enum onk_lexicon_t _)
{ return false; }

void __test__lex_harness_bad_hook(CuTest *tc)
{
    uint16_t failed_on = 0, ngenerated = 0;
    int8_t ret;

    enum onk_lexicon_t generated[2];
    enum onk_lexicon_t wrong[] = {
      ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, 0
    };

    ret = _onk_range_harness(wrong, bad_hook, &failed_on,
                       generated, &ngenerated, 2);

    CuAssertTrue(tc, ret == -4);
}

/***************
* test results *
***************/
bool test_handle_succeed(enum onk_lexicon_t tok)
{
    return tok == ONK_TRUE_TOKEN
        || tok == ONK_FALSE_TOKEN
        || tok == ONK_NULL_TOKEN;
}


void __test__lex_harness_succeed(CuTest *tc)
{
    int8_t ret;
    uint16_t failed_on = 0, ngenerated = 0;
    enum onk_lexicon_t generated[MAX_HARNESS];
    enum onk_lexicon_t answers[] = {
      ONK_TRUE_TOKEN, ONK_FALSE_TOKEN, ONK_NULL_TOKEN, 0
    };


    ret = _onk_range_harness(
        answers, test_handle_succeed, &failed_on,
        generated, &ngenerated, MAX_HARNESS);

    CuAssertIntEquals(tc, ret, 0);
    CuAssertIntEquals(tc, failed_on, 0);
    CuAssertIntEquals(tc, ngenerated, 3);
}

void __test__lex_harness_fail(CuTest *tc)
{
    uint16_t failed_on = 0, ngenerated = 0;
    int8_t ret;

    enum onk_lexicon_t generated[2];
    enum onk_lexicon_t wrong[] = {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, 0};

    ret = _onk_range_harness(
        wrong, test_handle_overflow, &failed_on,
        generated, &ngenerated, 2);

    CuAssertIntEquals(tc, ret, -1);
    CuAssertIntEquals(tc, failed_on, 0);

}

/********************
* use lexer harness *
*********************/

void __test__is_tok_unary_operator(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_NOT_TOKEN, ONK_TILDE_TOKEN, 0
    };

    LexRangeHarness(
        tc, "", answers,
        onk_is_tok_unary_operator);
}


void __test__is_tok_delimiter(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_SEMICOLON_TOKEN, ONK_COMMA_TOKEN,
      0
    };

    LexRangeHarness(
        tc, "", answers,
        onk_is_tok_unary_operator);
}

void __test__is_tok_brace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BRACE_OPEN_TOKEN,
      ONK_BRACE_CLOSE_TOKEN,
      ONK_BRACKET_OPEN_TOKEN,
      ONK_BRACKET_CLOSE_TOKEN,
      ONK_PARAM_CLOSE_TOKEN,
      ONK_PARAM_OPEN_TOKEN,
      ONK_HASHMAP_LITERAL_START_TOKEN, 0
    };

    LexRangeHarness(
        tc, "", answers,
        onk_is_tok_unary_operator);
}


void __test__is_tok_open_brace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BRACE_OPEN_TOKEN,
      ONK_BRACKET_OPEN_TOKEN,
      ONK_PARAM_OPEN_TOKEN,
      ONK_HASHMAP_LITERAL_START_TOKEN, 0
    };

    LexRangeHarness(
        tc, "", answers,
        onk_is_tok_unary_operator);
}


void __test__is_tok_close_brace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BRACE_OPEN_TOKEN,
      ONK_BRACKET_OPEN_TOKEN,
      ONK_PARAM_OPEN_TOKEN,
      ONK_HASHMAP_LITERAL_START_TOKEN, 0
    };

    LexRangeHarness(
        tc, "", answers,
        onk_is_tok_unary_operator);
}

void __test__is_tok_whitespace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_NEWLINE_TOKEN,
      ONK_WHITESPACE_TOKEN,
      0
    };

    LexRangeHarness(
        tc, "", answers,
        onk_is_tok_unary_operator);
}

void __test__is_tok_operator(CuTest *tc)
{
    enum onk_lexicon_t *answers = {0};

    LexRangeHarness(
        tc, "", answers,
        onk_is_tok_illegal);
}

void __test__is_tok_binop(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      0
    };

    LexRangeHarness(tc, "", answers, onk_is_tok_binop);
}

void __test__is_tok_block_keyword(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_IF_TOKEN, ONK_ELSE_TOKEN, ONK_DEF_TOKEN,
      ONK_FOR_TOKEN, ONK_WHILE_TOKEN, ONK_FROM_TOKEN,
      ONK_BREAK_TOKEN, ONK_CONTINUE_TOKEN, ONK_IMPL_TOKEN,
      ONK_IMPORT_TOKEN, ONK_RETURN_TOKEN, ONK_STRUCT_TOKEN,
      0
    };

    LexRangeHarness(tc, "", answers, onk_is_tok_block_keyword);
}

void __test__is_tok_unit(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_TRUE_TOKEN,
      ONK_FALSE_TOKEN,
      ONK_NULL_TOKEN,
      ONK_INTEGER_TOKEN,
      ONK_WORD_TOKEN,
      ONK_STRING_LITERAL_TOKEN,
      0
    };

    LexRangeHarness(tc, "", answers, onk_is_tok_unit);
}

/* void __test__is_tok_keyword(CuTest *tc) */
/* { */
    /* enum onk_lexicon_t answers[] = { */
    /*   0 */
    /* }; */

    //LexRangeHarness(tc, "", answers, onk_is_tok_keyword);
/* } */

void __test__is_tok_loopctl(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_CONTINUE_TOKEN,
      ONK_BREAK_TOKEN,
      0
    };

    LexRangeHarness(tc, "", answers, onk_is_tok_loopctlkw);
}

void __test__is_tok_asn_op(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BIT_OR_EQL,
      ONK_BIT_AND_EQL,
      ONK_BIT_NOT_EQL,
      ONK_MINUS_EQL_TOKEN,
      ONK_PLUSEQ_TOKEN,
      ONK_EQUAL_TOKEN,
      0
    };

    LexRangeHarness(tc, "", answers, onk_is_tok_asn_operator);
}

void __test__is_tok_group_modifier(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      0
    };

    LexRangeHarness(tc, "", answers, onk_is_tok_group_modifier);
}

void __test__is_tok_group_ident(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      0
    };

    LexRangeHarness(tc, "", answers, _onk_is_group);
}

void __test__print_tokens(CuTest *tc)
{
    static const char * no_name = "ONK_PTOKEN_UNKNOWN_TOKEN";
    const char * name;
    char msg[256];

    for(enum onk_lexicon_t i=__ONK_TOKEN_START; __ONK_TOKEN_END > i; i++)
    {
        name = onk_ptoken(i);
        if(memcmp(name, no_name, 24) != 0)
        {
            snprintf(msg, 256, "failed on %d", i);
            CuFail(tc, msg);
        }
    }
}


CuSuite* LexerHarnessUBTests(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, __test__lex_harness_empty_arr);
    SUITE_ADD_TEST(suite, __test__lex_harness_null_ptr);
    SUITE_ADD_TEST(suite, __test__lex_harness_overflow);
    SUITE_ADD_TEST(suite, __test__lex_harness_succeed);
    SUITE_ADD_TEST(suite, __test__lex_harness_fail);
    return suite;
}


CuSuite* LexerHarnessLogicTests(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__print_tokens);

    SUITE_ADD_TEST(suite, __test__is_tok_close_brace);
    SUITE_ADD_TEST(suite, __test__is_tok_open_brace);
    SUITE_ADD_TEST(suite, __test__is_tok_brace);

    SUITE_ADD_TEST(suite, __test__is_tok_unary_operator);
    SUITE_ADD_TEST(suite, __test__is_tok_binop);
    SUITE_ADD_TEST(suite, __test__is_tok_asn_op);

    /* SUITE_ADD_TEST(suite, __test__is_tok_keyword); */
    SUITE_ADD_TEST(suite, __test__is_tok_block_keyword);
    SUITE_ADD_TEST(suite, __test__is_tok_loopctl);

    SUITE_ADD_TEST(suite, __test__is_tok_delimiter);
    SUITE_ADD_TEST(suite, __test__is_tok_group_ident);
    return suite;
}
