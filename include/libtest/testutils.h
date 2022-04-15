#ifndef _HEADER__COMMON_TESTS_HELPER__
#define _HEADER__COMMON_TESTS_HELPER__

#include "libtest/CuTest.h"
#include "lexer.h"


void create_mock_tokens(
    struct onk_token_t * tokens,
    uint16_t n,
    enum onk_lexicon_t *tok
);

/**
 *  Assert tokens match the type found in answer
 * @param tc: CU-test suite
 * @param src: Source code
 * @param msg: Error Message
 * @param tokens: tokens to check against answers
 * @param answer: list of answers
*/
void onk_assert_tokens(
    CuTest *tc,
    const struct onk_token_t tokens[],
    const enum onk_lexicon_t answer[],
    const char *msg,
    const char *src_code,
    const char *file,
    uint16_t line
);

/**
 *  Assert tokens match the type found in answer
 * @param tc: CU-test suite
 * @param src: Source code
 * @param msg: Error Message
 * @param tokens: list of token pointers to check against answers
 * @param answer: list of answers
*/
void onk_assert_tokens_by_ref(
    CuTest *tc,
    const struct onk_token_t *tokens[],
    const enum onk_lexicon_t answer[],
    const char *msg,
    const char *src_code,
    const char *file,
    uint16_t line
);

#define OnkAssertTokens(tc, tokens, answer, msg, src_code)                 \
    onk_assert_tokens((tc), (tokens) (answer), (msg), (src_code), __FILE__, __LINE__)

#define OnkAssertRefTokens(tc, tokens, answer, msg, src_code)              \
    onk_assert_tokens_by_ref((tc), (tokens) (answer), (msg), (src_code), __FILE__, __LINE__)

#endif
