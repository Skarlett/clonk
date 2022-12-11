#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "onkstd/merge_sort.h"
#include "libtest/CuTest.h"
#include "libtest/assert.h"
#include "lexer.h"

const char * error_missing_answers(
    enum onk_lexicon_t *answers,
    uint16_t *missing,
    uint16_t nmissing
){
    const char *prefix="\n------------\nMissing from generated:  \n";
    char * message = malloc(2048);
    uint16_t message_sz = 2048;
    uint16_t message_len = 0;
    char tname_buf[64] = {0};
    uint16_t tname_len = 0;

    strcat(message, prefix);
    message_len = strlen(prefix) - 1;

    for(uint16_t k=0; nmissing > k; k++)
    {
        memset(tname_buf, 0, 64);
        tname_len = snprintf(tname_buf, 64, "[%s(answer index: %u)]\n", onk_ptoken(answers[missing[k]]), missing[k]);
        assert(tname_len);
        if(message_len + tname_len >= message_sz)
        {
            message = realloc(message, message_sz * 2);
            assert(message);
            message_sz *= 2;
        }
        strncat(message, tname_buf, message_sz - message_len);
        message_len += tname_len;
    };
    strncat(message, "\n------------", message_sz - message_len);
    return message;
}

struct _lexrng_linkedlist_node_t
{
    struct _lexrng_linkedlist_node_t *next;
    uint16_t i;
};

uint16_t error_extra_generated(
    enum onk_lexicon_t *answers,
    struct _lexrng_linkedlist_node_t *rootnode,
    uint16_t *missing
){
    struct _lexrng_linkedlist_node_t *stepnode = rootnode;
    uint16_t nmissing = 0;

    while(stepnode)
    {
        missing[nmissing] = answers[stepnode->i];
        stepnode = stepnode->next;
        nmissing += 1;
    }

    return nmissing;
}

void _lex_range_harness(
    CuTest *tc,
    const char *file,
    uint16_t line,
    handler handler,
    enum onk_lexicon_t *answers,
    uint16_t answers_len
){
    enum onk_lexicon_t tok = 0;
    enum onk_lexicon_t generated[PH_ONK_TOKEN_END+1];
    uint16_t missing[PH_ONK_TOKEN_END+1];

    uint8_t missing_idx = 0, gen_idx=0;
    int16_t contains_idx = 0;

    CuAssert(tc, "harness called with null pointer, or empty answers", answers_len != 0);
    CuAssert(tc, "harness called with null pointer, or empty answers", answers != 0);

    /* generate all tokens  */
    for(tok = PH_ONK_TOKEN_START; PH_ONK_TOKEN_END > tok; tok++)
    {
        if(handler(tok))
        {
            generated[gen_idx] = tok;
            gen_idx += 1;
        }
    }

    // add missing tokens (from answers) to the missing buffer
    for (uint16_t i=0; answers_len > i; i++)
    {
        contains_idx = onk_lexarr_contains(answers[i], generated, gen_idx);
        if (contains_idx == -1)
            missing[missing_idx++] = i;
    }

    if(missing_idx)
        CuFail_Line(
            tc,
            file,
            line,
            NULL,
            error_missing_answers(answers, missing, missing_idx)
        );
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
* test bad hook *
****************/
bool bad_hook(enum onk_lexicon_t _)
{ return false; }

/***************
* test results *
***************/
bool test_handle_succeed(enum onk_lexicon_t tok)
{
    return tok == ONK_TRUE_TOKEN
        || tok == ONK_FALSE_TOKEN
        || tok == ONK_NULL_TOKEN;
}

/* void __test__lex_harness_fail(CuTest *tc) */
/* { */
/*     uint16_t failed_on = 0, ngenerated = 0; */
/*     int8_t ret=0; */
/*     enum onk_lexicon_t generated[2]; */
/*     enum onk_lexicon_t wrong[] = {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, 0}; */
/*     ret = _onk_range_harness( */
/*         wrong, test_handle_overflow, &failed_on, */
/*         generated, &ngenerated, 2); */
/*     CuAssertIntEquals(tc, ret, -1); */
/*     CuAssertIntEquals(tc, failed_on, 0); */
/* } */

/********************
* use lexer harness *
*********************/

void __test__is_tok_unary_operator(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_NOT_TOKEN, ONK_TILDE_TOKEN, 0
    };

    LexRangeHarness(tc, onk_is_tok_unary_operator, answers, 2);
}

void __test__is_tok_delimiter(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_SEMICOLON_TOKEN, ONK_COMMA_TOKEN,
      0
    };

    LexRangeHarness(tc, onk_is_tok_delimiter, answers, 2);
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

      ONK_HASHMAP_LITERAL_START_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_brace,
        answers, 7
    );
}

void __test__is_tok_open_brace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BRACE_OPEN_TOKEN,
      ONK_BRACKET_OPEN_TOKEN,
      ONK_PARAM_OPEN_TOKEN,
      ONK_HASHMAP_LITERAL_START_TOKEN,
      0
    };

    LexRangeHarness(tc, onk_is_tok_open_brace, answers, 4);
}

void __test__is_tok_close_brace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BRACE_CLOSE_TOKEN,
      ONK_BRACKET_CLOSE_TOKEN,
      ONK_PARAM_CLOSE_TOKEN,
    };

    LexRangeHarness(
        tc,
        onk_is_tok_close_brace,
        answers, 3
    );
}

void __test__is_tok_whitespace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_NEWLINE_TOKEN,
      ONK_WHITESPACE_TOKEN,
    };

    LexRangeHarness(
        tc,
        onk_is_tok_whitespace,
        answers,
        2
    );
}

/* void __test__is_tok_operator(CuTest *tc) */
/* { */
/*     enum onk_lexicon_t *answers = {0}; */
/*     LexRangeHarness( */
/*         tc, "", answers, */
/*         onk_is_tok_illegal */
/*     ); */
/* } */

/* void __test__is_tok_binop(CuTest *tc) */
/* { */
/*     enum onk_lexicon_t answers[] = {0}; */
/*     LexRangeHarness(tc, "", answers, onk_is_tok_binop); */
/* } */

void __test__is_tok_block_keyword(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_IF_TOKEN, ONK_ELSE_TOKEN, ONK_DEF_TOKEN,
      ONK_FOR_TOKEN, ONK_WHILE_TOKEN, ONK_FROM_TOKEN,
      ONK_BREAK_TOKEN, ONK_CONTINUE_TOKEN, ONK_IMPL_TOKEN,
      ONK_IMPORT_TOKEN, ONK_RETURN_TOKEN, ONK_STRUCT_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_block_keyword,
        answers,
        12
    );
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

    LexRangeHarness(
        tc,
        onk_is_tok_unit,
        answers,
        6
    );
}

void __test__is_tok_keyword(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_keyword,
        answers,
        0
    );
}

void __test__is_tok_loopctl(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_CONTINUE_TOKEN,
      ONK_BREAK_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_loopctlkw,
        answers, 2
    );
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

    LexRangeHarness(
        tc,
        onk_is_tok_asn_operator,
        answers,
        6
    );
}

void __test__is_tok_group_modifier(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
       0,
    };
    LexRangeHarness(
        tc,
        onk_is_tok_group_modifier,
        answers,
        0
   );
}

void __test__is_tok_group_ident(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {0};
    LexRangeHarness(
        tc,
        onk_is_group,
        answers,
        0
   );
}

void __test__all_tokens_have_ptoken_impl(CuTest *tc)
{
    const char * no_name = onk_ptoken(PH_ONK_TOKEN_START);
    const char * name = 0;
    char msg[256];

    for(enum onk_lexicon_t i=PH_ONK_TOKEN_START; PH_ONK_TOKEN_END > i; i++)
    {
        /* roll over illegal tokens */
        if (onk_lexarr_contains(i, (enum onk_lexicon_t *)ILLEGAL_TOKENS, ILLEGAL_TOKENS_LEN) > -1)
            continue;

        name = onk_ptoken(i);
        if(name == no_name)
        {
            snprintf(msg, 256, "failed on idx %d [got: %s]", i, name);
            CuFail(tc, msg);
        }
    }
}

CuSuite* LexerHarnessLogicTests(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__all_tokens_have_ptoken_impl);

    SUITE_ADD_TEST(suite, __test__is_tok_close_brace);
    SUITE_ADD_TEST(suite, __test__is_tok_open_brace);
    SUITE_ADD_TEST(suite, __test__is_tok_brace);

    SUITE_ADD_TEST(suite, __test__is_tok_unary_operator);
    /* SUITE_ADD_TEST(suite, __test__is_tok_binop); */
    SUITE_ADD_TEST(suite, __test__is_tok_asn_op);

    SUITE_ADD_TEST(suite, __test__is_tok_keyword);
    SUITE_ADD_TEST(suite, __test__is_tok_block_keyword);
    SUITE_ADD_TEST(suite, __test__is_tok_loopctl);
    SUITE_ADD_TEST(suite, __test__is_tok_delimiter);
    SUITE_ADD_TEST(suite, __test__is_tok_group_ident);
    return suite;
}
