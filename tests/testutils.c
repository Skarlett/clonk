#include "CuTest.h"
#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/debug.h"
#include "../src/parser/lexer/helpers.h"
#include "../src/prelude.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

bool seq_eql_ty(
    const struct Token tokens[],
    const enum Lexicon lexicon[], usize len
){
    for (int i=0; len > i; i++) {
        if (tokens[i].type != lexicon[i]) {
            return false;
        }
    }
    return true;
}

bool __check_tokens_by_ref(
    const struct Token *tokens[],
    const enum Lexicon lexicon[], usize len
){
    for (usize i=0; len > i; i++) {
        if (tokens[i]->type != lexicon[i]) {
            return false;
        }
    }
    return true;
}

void AssertTokens(
    CuTest *tc,
    const char *file,
    int line,
    const char *msg,
    const struct Token tokens[],
    const enum Lexicon answer[],
    usize len
){
    char buf[2048];
    char got[512];
    char expected[512];
    sprintf_token_slice(tokens, len, got, 512);
    sprintf_lexicon_slice(answer, len, expected, 512);
    sprintf(buf, "%s\nexpected: \n%s\ngot: \n%s\n", msg, expected, got);

    CuAssert_Line(tc, file, line, buf, seq_eql_ty(tokens, answer, len) == 0);
}

void AssertTokensByRef(
    CuTest *tc,
    const char *file,
    int line,
    const char *msg,
    const struct Token *tokens[],
    const enum Lexicon answer[],
    usize len
){
    char buf[2048];
    char got[512];
    char expected[512];

    sprintf_token_slice_by_ref(tokens, len, got, 512);
    sprintf_lexicon_slice(answer, len, expected, 512);
    sprintf(buf, "%s\nexpected: \n%s\ngot: \n%s\n", msg, expected, got);
    CuAssert_Line(tc, file, line, buf,  __check_tokens_by_ref(tokens, answer, len) == 1);
}


void mk_toks(struct Token token[], const enum Lexicon ty[], usize ntoks) {
    for (usize i=0; ntoks > i; i++)
        token[i].type = ty[i];
}


void __test__check_tokens(CuTest* tc) {
     struct Token toks[8];

     const static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
        {INTEGER, INTEGER, DIV, INTEGER, ADD},
        {WORD, WORD, ADD, WORD, WORD, MUL, SUB},
        {INTEGER, INTEGER, MUL, INTEGER, ADD},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
    };

    uint8_t list_sz[] = {
        3, 5, 5, 6, 5, 5
    };

    for (uint8_t i=0; 6 > i; i++){
        mk_toks(toks, check_list[i], list_sz[i]);
        CuAssertTrue(tc, seq_eql_ty(toks, check_list[i], list_sz[i]) == true);
    }
}


CuSuite* TestUtilTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__check_tokens);
    return suite;
}