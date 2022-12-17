#include "libtest/CuTest.h"
#include "clonk.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

/**
 *  Assert tokens match the type found in answer
 * @param tc: CU-test suite
 * @param src: Source code
 * @param msg: Error Message
 * @param tokens: tokens to check against answers
 * @param answer: list of answers
*/
#define MAX_CHECK 128
#define MSGBUF_SZ 1024
static const char * const FMT = "%s:%u %sn\equality check failed on %u. " \
    "Expected <%s> got <%s>";

int8_t onk_assert_tokens(
    CuTest *tc,
    /* onk_vec_t<struct onk_token_t> */
    const struct onk_token_t tokens[],
    const enum onk_lexicon_t answer[],
    char * msg,
    const char *file,
    uint16_t line
)
{
    int16_t answer_len=0, token_len=0;
    long unsigned msg_offset = 0;
    char *msg2 = msg, *tmp=0;
    char msgbuf[MSGBUF_SZ];
    struct onk_token_t *token = 0;

    assert(answer);
    assert(tokens);

    for(answer_len=0; MAX_CHECK > answer_len; answer_len++)
        if(answer[answer_len] == 0)
            break;
        
    if(!answer_len)
      CuFail(tc, "No answers provided");

    for(token_len=0; MAX_CHECK > token_len; token_len++)
        if(tokens[token_len].type == ONK_EOF_TOKEN)
            break;
    
    memset(msgbuf, 0, MSGBUF_SZ);
    
    if (!msg2)
        msg2="n/a:";
    
    msg_offset = snprintf(
        msgbuf, MSGBUF_SZ,
        "%s: expected len <%u> got len <%u>",
        msg2, answer_len, token_len
    );
    
    strncat(msgbuf, "\n     got:  ", MSGBUF_SZ - msg_offset);
    msg_offset += 12;

    msg_offset += onk_tokarr_strncat_as_lexarr(
        msgbuf + msg_offset,
        MSGBUF_SZ - msg_offset,
        tokens,
        // TODO: FIXME
        token_len
    );

    strncat(msgbuf, "\nexpected: ", MSGBUF_SZ - msg_offset);
    msg_offset += 11;

    msg_offset += onk_lexarr_strncat(
        msgbuf,
        MSGBUF_SZ - msg_offset,
        answer,
        answer_len
    );

    if(answer_len != token_len)
        CuFail(tc, msgbuf);

    for (uint16_t i= 0; answer_len > i; i++)
    {
        token = &((struct onk_token_t *)tokens)[i];

        if(token->type != answer[i])
        {
            CuFail(tc, msgbuf);
            return -1;
        }
    }

    return 1;
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
