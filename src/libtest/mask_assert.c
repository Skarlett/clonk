#include <string.h>
#include "clonk.h"
#include "onkstd/vec.h"
#include "lexer.h"
#include "parser.h"

#include "libtest/masking.h"
#include "libtest/CuTest.h"

int16_t print_expect_line(
    CuTest *tc,
    char * buf,
    uint16_t nbuf,
    struct onk_stage_test *test,
    char * msg,
    uint16_t mismatched_idx
){
    char lexed_token[ONK_TOK_CHAR_SIZE];
    char expected_token[ONK_TOK_CHAR_SIZE];

    uint16_t spaces = 0;
    uint16_t underline = 0;
    uint16_t nwrote;

    char * tail;

    if(msg == 0)
        msg = "N/A";

    onk_snprint_token_type(
        lexed_token, ONK_TOK_CHAR_SIZE,
        test->source[mismatched_idx].type
    );
    onk_snprint_token_type(
        expected_token, ONK_TOK_CHAR_SIZE,
        test->expected[mismatched_idx]
    );

    for(uint16_t i = 0; mismatched_idx > i; i++)
        spaces += onk_token_len(&test->source[i]);

    underline = onk_token_len(&test->source[mismatched_idx]);

    nwrote = snprintf(
        buf, nbuf,
        "%s:%u (expected `%s` got `%s`)\n"    \
        "%s\n"                                              \
        "src: `%s`\n",
        test->fp, test->line,
        expected_token, lexed_token, msg,
        test->src_code
    );

    if(nwrote + spaces + underline + 1 > nbuf)
    {
        snprintf(buf, nbuf, "[Bad test] MessageBuffer overflowed. %s:%u\n", test->fp, test->line);
        CuFail(tc, buf);
        return -1;
    }

    tail = &buf[nwrote];
    memset(tail, ' ', spaces);
    nwrote += spaces;

    tail = &tail[nwrote];
    memset(tail, '~', underline);
    nwrote += underline + 1;

    tail[underline] = '\n';

    return nwrote;
}

int16_t onk_snprint_lex_err(
    CuTest *tc,
    char * buf,
    uint16_t nbuf,
    char * msg,
    struct onk_stage_test *test,
    uint16_t mismatched_idx
){
    uint16_t spaces = 0;
    uint16_t underline = 0;
    int16_t nwrote;

    uint16_t lex_sz =                                               \
        (onk_strlen_tok_arr(test->source, test->nsource) + 1) * sizeof(char);

    uint16_t expect_sz =                                                \
        (onk_strlen_lex_arr(test->expected, test->nexpected) + 1) * sizeof(char);

    char * lexicon_buf = malloc(lex_sz);
    char * expected_buf = malloc(expect_sz);
    char * tail;

    if(expect_sz + lex_sz > nbuf)
    {
        snprintf(buf, nbuf, "[Bad test] MessageBuffer overflowed. %s:%u\n", test->fp, test->line);
        CuFail(tc, buf);
        return -1;
    }

    nwrote = print_expect_line(
        tc,
        buf,
        nbuf,
        test,
        msg,
        mismatched_idx
    );

    if(nwrote == -1)
    {
        snprintf(buf, nbuf, "[Bad test] MessageBuffer overflowed. %s:%u\n", test->fp, test->line);
        CuFail(tc, buf);
        return -1;
    }

    for(uint16_t i = 0; mismatched_idx > i; i++)
        spaces += (strlen(onk_ptoken(test->expected[i])) || 1);

    underline = strlen(onk_ptoken(
        test->expected[mismatched_idx]
    ));

    nwrote += snprintf(
        &buf[nwrote], nbuf - nwrote,
        "  tokens: %s\n"                        \
        "expected: %s\n",
        lexicon_buf, expected_buf
    );

    tail = &buf[nwrote];

    if(nwrote + spaces + underline + 1 > nbuf - nwrote)
    {
        snprintf(buf, nbuf, "[Bad test] MessageBuffer overflowed. %s:%u\n", test->fp, test->line);
        CuFail(tc, buf);
        return -1;
    }

    memset(tail, ' ', spaces);
    nwrote += spaces;
    tail = &buf[nwrote];

    memset(tail, '~', underline);
    nwrote += underline;
    tail[nwrote] = '\n';

    free(lexicon_buf);
    free(expected_buf);

    return nwrote + 1;
}

int16_t kit_to_test(
    struct onk_stage_test *test,
    struct onk_test_mask_t *kit,
    uint16_t fmt_i
){

    uint16_t i;

    for(i=0; kit->narr > i; i++)
    {
        if(i >= test->expected_sz)
            return -1;

        switch(kit->arr[i].slot_type)
        {
            case onk_static_slot:
                test->expected[i] = kit->arr[i].data.static_tok;
                break;

            case onk_dynamic_slot:
                test->expected[i] = kit->arr[i].data.dyn_tok.arr[fmt_i];
                break;

            case onk_inspect_slot:
                if(test->ninspect >= ONK_TEST_INSP_SZ)
                    return -1;

                test->expected[i] = kit->arr[i].data.inspect.token.type;
                test->inspect_arr[test->ninspect] = i;
                test->ninspect += 1;
                break;

            default:
                return -1;
        }
    }

    return i + 1;
}

int8_t onk_init_test(
    struct onk_stage_test *test,
    struct onk_test_mask_t *kit,
    const char * src_code,
    struct onk_token_t *source_arr,
    uint16_t nsource,
    enum onk_lexicon_t *expected,
    uint16_t expected_sz,
    uint16_t iter,
    const char * fp,
    uint16_t line

) {
    int16_t flag = 0;
    test->source = source_arr;
    test->src_code = src_code;
    test->nsource = nsource;
    test->expected = expected;
    test->expected_sz = expected_sz;
    test->nexpected = 0;
    test->fp = fp;
    test->line = line;

    flag = kit_to_test(
        test,
        kit,
        iter
    );

    if(flag == -1)
        return -1;

    test->nexpected = flag;
    return 0;
}

void handle_overflow_msg(
    CuTest *tc,
    const char * msg,
    uint16_t nwrote,
    uint16_t line,
    const char *fp)
{
    char buf[512];
    const char *fmt_buf = "%s\nmsg overflowed (wrote: %u). %s:%u";
    snprintf(buf, 512, fmt_buf, msg, nwrote, line, fp);
    CuFail(tc, buf);
}

#define HandleOverflow(tc, msg, nmsg, source)   \
    handle_overflow_msg((tc), (msg), nmsg, __LINE__, __FILE__, #source)

int onk_assert_eq(
    CuTest *tc,
    char * buf,
    uint16_t nbuf,
    const char *fmt_str,
    uint16_t line,
    const char *fp,
    void *a, void *b,
    char *a_ident, char *b_ident,
    uint16_t size)
{
    char ident[128];
    snprintf(ident, 128, "%s == %s", a_ident, b_ident);

    if(snprintf(buf, nbuf, fmt_str, line, fp) > nbuf)
    {
        handle_overflow_msg(tc, ident, nbuf, line, fp);
        return -1;
    }
    CuAssert(tc, buf, memcmp(a, b, size) == 0);
    return 0;
}

int8_t _assert_narr_eql(
    CuTest *tc,
    char * buf,
    uint16_t nbuf,

    struct onk_test_mask_t *kit,
    struct onk_stage_test *test
){
    const char *fmt_buf;

    fmt_buf = "nsource doesn't match mask. %s:%u";
    if (snprintf(buf, nbuf, fmt_buf, test->line, test->fp) > nbuf)
    {
        handle_overflow_msg(tc, buf, nbuf, test->line, test->fp);
        return -1;
    }

    fmt_buf = "nexpected doesn't match mask. %s:%u";
    if(snprintf(buf, nbuf, fmt_buf, test->line, test->fp) > nbuf)
    {
        handle_overflow_msg(tc, buf, nbuf, test->line, test->fp);
        return -1;
    }

    CuAssert(tc, buf, test->nexpected == kit->narr);
    return 0;
}

int16_t match_type(
    CuTest *tc,
    char * buf,
    uint16_t nbuf,
    struct onk_stage_test *test,
    struct onk_test_mask_t *kit
)
{
    const char *msg;
    int16_t wrote;

    for(uint16_t i=0; test->nsource > i; i++)
    {
        if(kit->ignore_whitespace && onk_is_tok_whitespace(test->source[i].type))
            continue;

        switch(kit->arr[i].slot_type)
        {
            case onk_static_slot:
                msg = "Token did not match STATIC slot.";
                break;
            case onk_dynamic_slot:
                msg = "Token did not match DYNAMIC slot.";
                break;
            case onk_inspect_slot:
                msg = "Token did not match INSPECT slot.";
                break;
            default:
                snprintf(buf, 512, "Unhandled condition (%s:%u)", test->fp, test->line);
                CuFail(tc, buf);
                return -1;
        }

        wrote = onk_snprint_lex_err(
            tc,
            buf,
            nbuf,
            (char *)msg,
            test,
            i
        );

        if(wrote == -1 || test->expected[i] != test->source[i].type)
        {
            CuFail(tc, buf);
            return -1;
        }
    }

    return 0;
}

int8_t handle_inspect_slot(
    struct onk_desc_inspect_token_t *insp,
    struct onk_token_t *input)
{
    uint8_t flags = 0;
    struct onk_token_t *tok;

    flags = insp->ignore_flags;
    tok = &insp->token;

    if((flags & FINSP_START && tok->start != input->start)
      || (flags & FINSP_END && tok->end != input->end)
      || (flags & FINSP_SEQ && tok->seq != input->seq))
      return -1;
    return 0;
}

int8_t match_inspection(
    CuTest *tc,
    struct onk_stage_test *test,
    struct onk_test_mask_t *kit
)
{
    char msg[512];
    char got[128];
    char expected[128];

    struct onk_desc_inspect_token_t *insp;
    uint16_t idx = 0;

    for (uint8_t i=0; test->ninspect > i; i++)
    {
        idx = test->inspect_arr[i];
        insp = &kit->arr[idx].data.inspect;

        if(handle_inspect_slot(insp, &test->source[idx]) == 0)
        {
            onk_snprint_token(got, 128, &test->source[idx]);
            onk_snprint_token(expected, 128, &insp->token);
            snprintf(
                msg, 512,
                "%s:%u failed inspection."       \
                "\nExpected: %s"                 \
                "\nGot: %s"                      \
                "\nmask: %u"                     \
                "\nindex: %u",
                test->fp, test->line, expected, got, insp->ignore_flags, idx
            );

            CuFail(tc, msg);
            return -1;
        }
    }

    return 0;
}

int8_t onk_assert_match(
    CuTest *tc,
    struct onk_test_mask_t *kit,
    struct onk_token_t *input,
    uint16_t ninput,
    uint16_t iter,

    const char * src_code,
    char * filepath,
    uint16_t line
){
    char buf[512];
    enum onk_lexicon_t expected[64];
    struct onk_stage_test test;

    onk_init_test(
        &test, kit,
        src_code,
        input, ninput,
        expected, 64,
        iter,
        filepath, line
    );

    if(_assert_narr_eql(tc, buf, 512, kit, &test) == -1)
    {
        CuFail(tc, buf);
        return -1;
    }

    else if(match_type(tc, buf, 512, &test, kit) != 0)
    {
        CuFail(tc, buf);
        return -1;
    }


    return match_inspection(tc, &test, kit);
}

int8_t onk_assert_tokenize(
    CuTest *tc,
    struct onk_test_mask_t *kit,
    struct onk_lexer_input_t *lexer_input,
    struct onk_lexer_output_t *lexer_output,
    uint16_t iter,
    char * fp,
    uint16_t line
){
    int ret;
    char buf[512];

    ret = onk_tokenize(lexer_input, lexer_output);

    if(ret != 0)
    {
        snprintf(buf, 512, "tokenize failed %s:%u", fp, line);
        CuFail(tc, buf);
        return -1;
    }

    return onk_assert_match(
        tc,
        kit,
        lexer_output->tokens.base,
        lexer_input->tokens.len,
        iter,
        lexer_input->src_code,
        fp,
        line
    );
}

int8_t onk_assert_postfix(
    CuTest *tc,
    struct onk_test_mask_t *kit,
    struct onk_parser_input_t *input,
    struct onk_parser_output_t *output,
    uint16_t i,
    char * fp,
    uint16_t line
){
    int ret;
    char buf[512];

    snprintf(buf, 512, "Failed parsing stage %s:%u", fp, line);
    ret = onk_parse(input, output);

    if(ret != 0)
    {
        snprintf(buf, 512, "tokenize failed %s:%u", fp, line);
        CuFail(tc, buf);
        return -1;
    }

    return onk_assert_match(
        tc,
        kit,
        output->postfix.base,
        input->tokens.len,
        i,
        input->src_code,
        fp,
        line
    );
}

int8_t onk_assert_parse_stage(
    CuTest *tc,
    struct onk_test_mask_t *lexer,
    struct onk_test_mask_t *parser,
    uint16_t fmt_i,
    char *fp,
    uint16_t line
) {
    struct onk_lexer_input_t lexer_input;
    struct onk_lexer_output_t lexer_output;

    struct onk_parser_input_t parser_input;
    struct onk_parser_output_t parser_output;

    int8_t ret;

    ret = onk_assert_tokenize(
        tc,
        lexer,
        &lexer_input,
        &lexer_output,
        fmt_i,
        fp,
        line
    );

    if (ret == -1)
        return -1;

    onk_parser_input_from_lexer_output(&lexer_output, &parser_input, false);

    return onk_assert_postfix(
        tc,
        parser,
        &parser_input,
        &parser_output,
        fmt_i,
        fp,
        line
    );
}

