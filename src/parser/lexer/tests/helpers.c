#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "onkstd/merge_sort.h"
#include "libtest/CuTest.h"
#include "libtest/assert.h"
#include "lexer.h"


#define MAX_HARNESS 64

typedef bool (handler)(enum onk_lexicon_t);

int8_t _onk_range_harness(
    enum onk_lexicon_t *answers,
    handler handler,
    uint16_t *failed_on,
    enum onk_lexicon_t *generated_answers,
    uint16_t ngenerated)
{
    enum onk_lexicon_t tok;
    uint8_t answers_ctr = 0, idx=0;

    for (answers_ctr=0; MAX_HARNESS > answers_ctr; answers_ctr++)
        if(answers[answers_ctr] == 0)
            break;

    for(tok = __ONK_TOKEN_START; __ONK_TOKEN_END > tok; tok++)
    {
        if(handler(tok))
        {
            generated_answers[idx] = tok;
            idx += 1;
            if (idx >= ngenerated) return -2;
        }
    }

    if (answers_ctr != idx)
        return -3;

    /* only answers need to be sorted,
     * `generated_answers` will be sorted already */
    onk_merge_sort_u16((uint16_t *)answers, 0, answers_ctr);

    for(uint16_t i=0; answers_ctr > i; i++)
    {
        if(generated_answers[i] != answers[i])
        {
            *failed_on = i;
            return -1;
        }
    }
    return 0;
}


void onk_range_harness(
    CuTest *tc,
    char * msg,
    enum onk_lexicon_t *answers,
    handler handler,
    const char * handler_name,
    const char * fp,
    uint16_t line
)
{
    uint16_t failed_on = 0;
    char msg_buf[512];
    char header[128];
    enum onk_lexicon_t generated[64];
    int8_t ret;

    ret = _onk_range_harness(
        answers, handler, &failed_on, generated, 64);

    snprintf(
        header,
        128,
        "%s\n%s:%u [%s][%u] ",
        msg, fp, line, handler_name, failed_on
    );

    switch (ret)
    {
        case 0:
            return;

        case -1:
            snprintf(
                msg_buf,
                512,
                "%s expected <%s>, got <%s> on idx: %u\n",
                header,
                onk_ptoken(generated[failed_on]),
                onk_ptoken(answers[failed_on]),
                failed_on
            );
            CuFail(tc, msg_buf);
            return;

        case -2:
            snprintf(
                msg_buf,
                512,
                "%s generated tokens overflowed\n",
                header
            );
            CuFail(tc, msg_buf);
            break;

        case -3:
            snprintf(
                msg_buf,
                512,
                "%s length-equality failed\n",
                header
            );
            CuFail(tc, msg_buf);
            break;

        default:
            CuFail(tc, "unreachable code triggered");
            return;
    }

}

#define LexRangeHarness(tc, msg, answers, handler) \
    onk_range_harness((tc), (msg), (answers), (handler), (#handler), __FILE__, __LINE__)




void __test__lex_range_harness(CuTest *tc, struct onk_test_buffers *_)
{
    char msgbuf[512];
    jmp_buf jmp;
    CuTest dummy;
    dummy.jumpBuf = &jmp;

    if(setjmp(jmp) == 0)
    {
        LexRangeHarness(&dummy, "NA", )
    }
    dummy.jumpBuf = 0;


    CuAssertTrue(tc, dummy.failed == 0)
}


void __test__is_tok_unit(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_WORD_TOKEN, ONK_INTEGER_TOKEN,
      ONK_STRING_LITERAL_TOKEN, ONK_TRUE_TOKEN,
      ONK_FALSE_TOKEN, ONK_NULL_TOKEN
    };

    LexRangeHarness(
        tc,
        "failed tok-unit",
        answers,
        onk_is_tok_unit
    );
}

void __test__is_tok_operator(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
    };

    LexRangeHarness(
        tc,
        "failed tok-unit",
        answers,
        onk_is_tok_unit
    );
}


void __test__is_tok_group()
{

}

void __test__is_token_whitespace()
{

}
