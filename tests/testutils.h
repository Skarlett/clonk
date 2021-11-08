#ifndef _HEADER__COMMON_TESTS_HELPER__
#define _HEADER__COMMON_TESTS_HELPER__

#include "CuTest.h"
#include "../src/prelude.h"
#include "../src/parser/lexer/lexer.h"

int8_t seq_eql_ty(struct Token tokens[], enum Lexicon lexicon[], usize len);
int8_t __check_tokens_by_ref(struct Token *tokens[], enum Lexicon lexicon[], usize len);

void AssertTokens(
    CuTest *tc,
    const char *file,
    int line,
    const char *msg,
    const struct Token tokens[],
    const enum Lexicon answer[],
    usize len
);

void AssertTokensByRef(
    CuTest *tc,
    const char *file,
    int line,
    const char *msg,
    const struct Token *tokens[],
    const enum Lexicon answer[],
    usize len
);


#define AssertTokensByRef(tc, msg, tokens, answer, len) AssertTokensByRef((tc), __FILE__, __LINE__, (msg), (tokens), (answer), (len))
#define AssertTokens(tc, msg, tokens, answer, len) AssertTokens((tc), __FILE__, __LINE__, (msg), (tokens), (answer), (len))

#endif
