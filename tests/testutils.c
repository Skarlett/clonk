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
    const enum Lexicon lexicon[]
){
    for (usize i=0 ;; i++) {
        if(lexicon[i] == 0)
            break;
        
        else if (tokens[i].type != lexicon[i])
            return false;
    }
    return true;
}

bool seq_eql_ty_by_ref(
    const struct Token *tokens[],
    const enum Lexicon lexicon[]
){
    for (usize i=0 ;; i++) {
        if (lexicon[i] == 0)
            break;
        else if (tokens[i]->type != lexicon[i])
            return false;
    }
    return true;
}

#define FMT_STR "%s\nexpected: \n%s\ngot: \n%s\nsrc:\n%s\n"

void AssertTokens(
    CuTest *tc,
    const char *file,
    int line,
    const char *msg,
    const char *source_code,
    const struct Token tokens[],
    const enum Lexicon answer[],
    usize len
){
    char buf[2048];
    char got[512];
    char expected[512];

    sprintf_token_slice(tokens, len, got, 512);
    sprintf_lexicon_slice(answer, len, expected, 512);
    sprintf(buf, FMT_STR, msg, expected, got, source_code);

    CuAssert_Line(tc, file, line, buf, seq_eql_ty(tokens, answer) == 0);
}

void AssertTokensByRef(
    CuTest *tc,
    const char *file,
    int line,
    const char *msg,
    const char *source_code,
    const struct Token *tokens[],
    const enum Lexicon answer[],
    usize len
){
    char buf[2048];
    char got[512];
    char expected[512];

    sprintf_token_slice_by_ref(tokens, len, got, 512);
    sprintf_lexicon_slice(answer, len, expected, 512);
    sprintf(buf, FMT_STR, msg, expected, got, source_code);

    CuAssert_Line(tc, file, line, buf, seq_eql_ty_by_ref(tokens, answer) == 1);
}

void mk_toks(struct Token token[], const enum Lexicon ty[]) {
  for (usize i = 0 ;; i++)
    if (ty[i] == 0)
      break;
    else
      token[i].type = ty[i];
}

void __test__check_tokens(CuTest* tc) {
     struct Token toks[8];

     const static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, INTEGER, MUL, ADD, 0},
        {INTEGER, INTEGER, DIV, INTEGER, ADD, 0},
        {WORD, WORD, ADD, WORD, WORD, MUL, SUB, 0},
        {INTEGER, INTEGER, MUL, INTEGER, ADD, 0},
        {INTEGER, INTEGER, INTEGER, MUL, ADD, 0},
    };


    for (uint8_t i=0; 6 > i; i++){
        mk_toks(toks, check_list[i]);
        CuAssertTrue(tc, seq_eql_ty(toks, check_list[i]) == true);
    }
}


CuSuite* TestUtilTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__check_tokens);
    return suite;
}