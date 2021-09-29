#ifndef _HEADER__COMMON_TESTS_HELPER__
#define _HEADER__COMMON_TESTS_HELPER__
#include "CuTest.h"
#include "../src/prelude.h"
#include "../src/parser/lexer/lexer.h"

int __check_tokens(struct Token tokens[], enum Lexicon lexicon[], usize len);
int __check_tokens_by_ref(struct Token *tokens[], enum Lexicon lexicon[], usize len);

void AssertTokens(CuTest *tc, struct Token tokens[], enum Lexicon lexicon[], usize len);
#endif
