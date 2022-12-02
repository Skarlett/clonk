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
#define MAX_CHECK 512
static const char * const FMT = "%s:%u %sn\equality check failed on %u. " \
    "Expected <%s> got <%s>";

int8_t onk_assert_tokens(
    CuTest *tc,
    const struct onk_token_t tokens[],
    const enum onk_lexicon_t answer[],
    char * msg,
    const char *file,
    uint16_t line
)
{
    uint16_t i=0;
    char buf[512];

    for(i=0; MAX_CHECK > i; i++)
    {
        if(tokens[i].type != answer[i])
        {
            snprintf(buf, 512, FMT, file, line, msg, i,
                     onk_ptoken(answer[i]), onk_ptoken(tokens[i].type));
            CuFail(tc, buf);
            return -1;
        }
    }

    return 0;
}


int8_t onk_assert_tokens_by_ref(
    CuTest *tc,
    const struct onk_token_t *tokens[],
    const enum onk_lexicon_t answer[],
    char * msg,
    const char *file,
    uint16_t line
){
    uint16_t i=0;
    char buf[512];
    for(i=0; MAX_CHECK > i; i++)
    {
        if(tokens[i]->type != answer[i])
        {
            snprintf(
                buf, 512, FMT, file, line, msg, i,
                onk_ptoken(tokens[i]->type),
                onk_ptoken(answer[i])
            );
            CuFail(tc, buf);
            return -1;
        }
    }

    return MAX_CHECK > i ? -1 : 0;
}

void create_mock_tokens(struct onk_token_t * tokens, uint16_t n, enum onk_lexicon_t *tok)
{
    for(uint16_t i=0; n > i; i++)
    {
        tokens[i].start = 0;
        tokens[i].end = 0;
        tokens[i].seq = 0;
        tokens[i].type = tok[i];
    }
}
