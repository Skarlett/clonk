#include <string.h>
#include "onkstd/vec.h"
#include "lexer.h"
#include "parser.h"

#include "libtest/tokens.h"
#include "libtest/CuTest.h"

#define ONK_TEST_INSP_SZ 64
struct onk_test {
    const char * src_code;
    const char * fp;

    struct onk_token_t *source; // lexed||parsed
    enum onk_lexicon_t *expected;

    uint16_t inspect_arr[ONK_TEST_INSP_SZ];
    uint8_t ninspect;

    //uint16_t iter;
    //uint16_t fmt_i;
    uint16_t nsource;
    uint16_t source_sz;

    uint16_t nexpected;
    uint16_t expected_sz;

    uint16_t line;
};


int16_t print_expect_line(
    char * buf,
    uint16_t nbuf,
    struct onk_test *test,
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
        "[%s] failed at: L%u (expected `%s` got `%s`)\n"    \
        "%s\n"                                              \
        "src: `%s`\n",
        test->fp, test->line,
        expected_token, lexed_token, msg,
        test->src_code
    );

    if(nwrote + spaces + underline + 1 > nbuf)
        return -1;

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
    char * buf,
    uint16_t nbuf,
    char * msg,
    struct onk_test *test,
    uint16_t mismatched_idx
){
    uint16_t spaces = 0;
    uint16_t underline = 0;
    int16_t nwrote;

    uint16_t lex_sz =                                               \
        (onk_str_len_token_arr(test->source, test->nsource) + 1) * sizeof(char);

    uint16_t expect_sz =                                                \
        (onk_str_len_lexicon_arr(test->expected, test->nexpected) + 1) * sizeof(char);

    char * lexicon_buf = malloc(lex_sz);
    char * expected_buf = malloc(expect_sz);
    char * tail;

    nwrote = print_expect_line(
        buf,
        nbuf,
        test,
        msg,
        mismatched_idx
    );

    if(nwrote == -1)
        return -1;

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
        return -1;

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
      || (flags & FINSP_SEQ && tok->seq != input->seq)
      || (flags & FINSP_TYPE && tok->type != input->type))
      return -1;
    return 0;
}


int16_t kit_to_test(
    struct onk_test *test,
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
    struct onk_test *test,
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

void test_kit_narr(
    CuTest *tc,
    char * buf,
    struct onk_test_mask_t *kit,
    struct onk_test *test
){
    const char *fmt_buf;

    fmt_buf = "nsource doesn't match mask. %s:%u";
    snprintf(buf, 512, fmt_buf, test->line, test->fp);
    CuAssert(tc, buf, test->nsource == kit->narr);

    fmt_buf = "nexpected doesn't match mask. %s:%u";
    snprintf(buf, 512, fmt_buf, test->line, test->fp);
    CuAssert(tc, buf, test->nexpected == kit->narr);
}

void match_type(
    CuTest *tc,
    char * buf,
    uint16_t nbuf,
    struct onk_test *test,
    struct onk_test_mask_t *kit
)
{
    const char *msg;

    for(uint16_t i=0; test->nsource > i; i++)
    {
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
                return;
        }

        onk_snprint_lex_err(
            buf,
            nbuf,
            msg,
            test,
            i
        );

        CuAssert(tc, buf, test->expected[i] == test->source[i].type);
        memset(buf, 0, 512);
    }
}

void match_inspection(
    CuTest *tc,
    struct onk_test *test,
    struct onk_test_mask_t *kit
)
{
    uint16_t idx = 0;
    for (uint8_t i=0; test->ninspect > i; i++)
    {

        idx = test->inspect_arr[i];
        handle_inspect_slot(
            &kit->arr[idx].data.inspect,
            &test->source[idx]
        );

    }
}

void onk_assert_match(
    CuTest *tc,
    struct onk_test_mask_t *kit,
    struct onk_token_t *input,
    uint16_t ninput,
    uint16_t iter,

    const char * src_code,
    char * filepath,
    uint16_t line
){
    enum onk_lexicon_t expected[64];
    struct onk_test test;
    char buf[512];

    const char * fmt_buf = 0, *msg = 0;

    onk_init_test(
        &test, kit,
        src_code,
        input, ninput,
        expected, 64,
        iter,
        filepath, line
    );

    test_kit_narr(tc, buf, kit, &test);
    memset(buf, 0, 512);
    match_type(
        tc,
        buf,
        512,
        &test,
        kit
    );

    match_inspection(tc, &test, kit);
}

void onk_assert_tokenize(
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

    snprintf(buf, 512, "tokenize failed %s:%u", fp, line);
    ret = onk_tokenize(lexer_input, lexer_output);
    CuAssert(tc, buf, ret == 0);

    onk_assert_match(
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

void onk_assert_postfix(
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
    CuAssert(tc, buf, ret == 0);

    onk_assert_match(
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


void onk_assert_parse_stage(
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

    onk_assert_tokenize(
        tc,
        lexer,
        &lexer_input,
        &lexer_output,
        fmt_i,
        fp,
        line
    );

    onk_parser_input_from_lexer_output(
        &lexer_output, &parser_input, false
    );

    onk_assert_postfix(
        tc,
        parser,
        &parser_input,
        &parser_output,
        fmt_i,
        fp,
        line
    );
}

