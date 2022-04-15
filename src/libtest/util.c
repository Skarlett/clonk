#include "libtest/CuTest.h"
#include "clonk.h"
#include "lexer.h"

/**
 *  Assert tokens match the type found in answer
 * @param tc: CU-test suite
 * @param src: Source code
 * @param msg: Error Message
 * @param tokens: tokens to check against answers
 * @param answer: list of answers
*/
#define MAX_CHECK 2048
static const char * fmt_str = "%s:%u %sn\equality check failed on %u";

int8_t onk_assert_tokens(
    CuTest *tc,
    const struct onk_token_t tokens[],
    const enum onk_lexicon_t answer[],
    char * msg,
    const char *file,
    uint16_t line
)
{
    uint16_t i;
    char buf[512];


    for(i=0; MAX_CHECK > i; i++)
    {
        if(tokens[i].type == answer[i])
        {
            snprintf(buf, 512, fmt_str, file, line, msg, i);
            CuFail(tc, buf);
            return -1;
        }
    }

    return MAX_CHECK > i ? -1 : 0;
}


int8_t onk_assert_tokens_by_ref(
    CuTest *tc,
    const struct onk_token_t *tokens[],
    const enum onk_lexicon_t answer[],
    char * msg,
    const char *file,
    uint16_t line
)
{
    uint16_t i;
    char buf[512];

    for(i=0; MAX_CHECK > i; i++)
    {
        if(tokens[i]->type == answer[i])
        {
            snprintf(buf, 512, fmt_str, file, line, msg, i);
            CuFail(tc, buf);
            return -1;
        }
    }

    return MAX_CHECK > i ? -1 : 0;
}
