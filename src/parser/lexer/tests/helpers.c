#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "onkstd/merge_sort.h"
#include "libtest/CuTest.h"
#include "libtest/assert.h"
#include "lexer.h"

void __test__is_tok_unit(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_WORD_TOKEN, ONK_INTEGER_TOKEN,
      ONK_STRING_LITERAL_TOKEN, ONK_TRUE_TOKEN,
      ONK_FALSE_TOKEN, ONK_NULL_TOKEN
    };

    LexRangeHarness(
        tc,
        "failed tok-unit",
        answers,
        onk_is_tok_unit
    );
}

void __test__is_tok_operator(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {

};

    LexRangeHarness(
        tc,
        "failed tok-unit",
        answers,
        onk_is_tok_unit
    );
}


void __test__is_tok_group()
{

}

void __test__is_token_whitespace()
{

}


CuSuite* CuLexerHarnessInput(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, __test__is_tok_unit);
    SUITE_ADD_TEST(suite, __test__is_tok_operator);

    return suite;
}
