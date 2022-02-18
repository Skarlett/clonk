#ifndef _HEADER__COMMON_TESTS_HELPER__
#define _HEADER__COMMON_TESTS_HELPER__

#include "CuTest.h"
#include "../src/prelude.h"
#include "../src/parser/lexer/lexer.h"

void onk_assert_tokens(
    CuTest *tc,
    const char *src_code,
    const char *file,
    const char *msg,
    const struct onk_token_t tokens[],
    const enum onk_lexicon_t answer[]
);

void onk_assert_tokens_by_ref(
    CuTest *tc,
    const char *src_code,
    const char *file,
    const char *msg,
    const struct onk_token_t *tokens[],
    const enum onk_lexicon_t answer[]
);


/**
 *  Assert tokens match the type found in answer 
 * @param tc: CU-test suite
 * @param src: Source code
 * @param msg: Error Message
 * @param tokens: tokens to check against answers 
 * @param answer: list of answers
 */

/**
 *  Assert tokens match the type found in answer 
 * @param tc: CU-test suite
 * @param src: Source code
 * @param msg: Error Message
 * @param tokens: list of token pointers to check against answers 
 * @param answer: list of answers
 */

#endif
