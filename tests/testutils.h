#ifndef _HEADER__COMMON_TESTS_HELPER__
#define _HEADER__COMMON_TESTS_HELPER__

#include "CuTest.h"
#include "../src/prelude.h"
#include "../src/parser/lexer/lexer.h"

void AssertTokens(
    CuTest *tc,
    const char *src_code,
    const char *file,
    int line,
    const char *msg,
    const struct Token tokens[],
    const enum Lexicon answer[]
);

void AssertTokensByRef(
    CuTest *tc,
    const char *src_code,
    const char *file,
    int line,
    const char *msg,
    const struct Token *tokens[],
    const enum Lexicon answer[]
);


/**
 *  Assert tokens match the type found in answer 
 * @param tc: CU-test suite
 * @param src: Source code
 * @param msg: Error Message
 * @param tokens: tokens to check against answers 
 * @param answer: list of answers
 */
#define AssertTokensByRef(tc, src, msg, tokens, answer) AssertTokensByRef((tc), (src), __FILE__, __LINE__, (msg), (tokens), (answer))

/**
 *  Assert tokens match the type found in answer 
 * @param tc: CU-test suite
 * @param src: Source code
 * @param msg: Error Message
 * @param tokens: list of token pointers to check against answers 
 * @param answer: list of answers
 */
#define AssertTokens(tc, src, msg, tokens, answer) AssertTokens((tc), (src), __FILE__, __LINE__, (msg), (tokens), (answer))

#endif
