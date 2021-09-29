#include "CuTest.h"
#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/debug.h"
#include "../src/parser/lexer/helpers.h"
#include "../src/prelude.h"
#include <stdio.h>

int __check_tokens(struct Token tokens[], enum Lexicon lexicon[], usize len){
    for (int i=0; len > i; i++) {
        if (tokens[i].type != lexicon[i]) {
            return 0;
        }
    }
    return 1;
}

int __check_tokens_by_ref(struct Token *tokens[], enum Lexicon lexicon[], usize len){
    for (usize i=0; len > i; i++) {
        if (tokens[i]->type != lexicon[i]) {
            return 0;
        }
    }

    return 1;
}

void AssertTokens(CuTest *tc, struct Token tokens[], enum Lexicon answer[], usize len) {
    char msg[2048];
    char got[512];
    char expected[512];

    sprintf_token_slice(tokens, len, got, 512);
    sprintf_lexicon_slice(answer, len, expected, 512);
    sprintf(msg, "expected: \n%s\ngot: \n%s\n", expected, got);

    CuAssert(tc, msg, __check_tokens(tokens, answer, len) == 0);
}