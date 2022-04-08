#include <string.h>
#include "onkstd/vec.h"
#include "lexer.h"
#include "parser.h"

#include "libtest/tokens.h"
#include "libtest/CuTest.h"

/*
 * copies (`enum onk_lexicon_t *tok`) each item into
 * `onk_tk_token_match_t` as a static option.
*/
int8_t onk_desc_add_static_slot(
    struct onk_tk_token_match_t * mold,
    enum onk_lexicon_t *tok,
    uint16_t nitems)
{
    struct onk_desc_token_t dtok;

    if(mold->sarr > mold->narr)
        return -1;

    for(uint16_t i=0; nitems > i; i++)
    {
        if(tok[i] == 0)
            break;

        dtok.slot_type = onk_static_slot;
        dtok.data.static_tok = tok[i];

        mold->arr[mold->narr] = dtok;
        mold->narr += 1;
    }

    return 1;
}

int8_t onk_desc_add_inspect_slot(
    struct onk_tk_token_match_t *kit,
    struct onk_desc_inspect_token_t *inspect
){
    struct onk_desc_token_t descriptor;

    if(kit->narr >= kit->sarr)
      return -1;

    descriptor.slot_type = onk_inspect_slot;
    descriptor.data.inspect = *inspect;

    kit->arr[kit->narr] = descriptor;
    kit->narr += 1;
    return 0;
}

int onk_desc_add_dynamic_slot(
    struct onk_tk_token_match_t * kit,
    enum onk_lexicon_t *answers,
    uint16_t nanswers)
{
    struct onk_desc_token_t *dtok;

    if(kit->sarr > kit->narr)
        return -1;

    dtok = &kit->arr[kit->narr];
    kit->narr += 1;

    dtok->data.dyn_tok.arr = answers;
    dtok->data.dyn_tok.narr = nanswers;

    return 1;
}

int8_t onk_desc_add_static_repeating_slot(
    struct onk_tk_token_match_t *kit,
    enum onk_lexicon_t tok,
    uint16_t ntimes
)
{
    bool overflow = false;
    struct onk_desc_token_t descriptor;
    struct onk_desc_token_t *ptr;

    if(onk_add_u16(kit->narr, ntimes, &overflow) >= kit->sarr
       && overflow == 0)
        return -1;

    descriptor.slot_type = onk_static_slot;
    descriptor.data.static_tok = tok;

    ptr = &kit->arr[kit->narr];
    kit->narr += ntimes;

    for(uint16_t i=0; ntimes > i; i++)
        ptr[i] = descriptor;

    return 1;
}

int8_t onk_desc_add_repeating_slot(
    struct onk_tk_token_match_t *kit,
    struct onk_desc_token_t *tok,
    uint16_t ntimes
)
{
    bool overflow = false;
    struct onk_desc_token_t *ptr;

    if(onk_add_u16(kit->narr, ntimes, &overflow) >= kit->sarr
       && overflow == 0)
        return -1;

    ptr = &kit->arr[kit->narr];
    kit->narr += ntimes;

    memcpy(
        &kit->arr[kit->narr],
        tok,
        sizeof(struct onk_desc_token_t) * ntimes
    );

    return 1;
}

void onk_match_token_init(
    struct onk_tk_token_match_t *kit,
    struct onk_desc_token_t *buffer,
    uint16_t buffer_sz
)
{
    kit->arr = buffer;
    kit->narr = buffer_sz;
}


int16_t type_error_message(
    char * buf,
    uint16_t nbuf,
    char * src_code,
    struct onk_token_t *lexed,
    enum onk_lexicon_t *expected,
    uint16_t mismatched_idx,
    char * fp,
    uint16_t line)
{
    char lexicon_buf[256];

    uint16_t spaces = 0;
    uint16_t underline = 0;
    uint16_t remaining_bytes = nbuf;

    snprintf(buf, remaining_bytes, "[%s] failed at: L%d\n" \
             "src: `%s`\n"                      \
             "tokens: %s",
             fp, line, src_code, lexicon_buf);


    strncat();

    remaining_bytes = _onk_snprint_lexicon_arr(
        buf, remaining_bytes, dtok->data.static_tok);
}

int16_t token_error_message(
    char * buf,
    uint16_t nbuf,
    char * src_code,
    struct onk_token_t *lexed,
    enum onk_lexicon_t *expected,
    char * fp,
    uint16_t line)
{
    snprintf(buf, nbuf, "[%s] failed at: L%d\n" \
             "src: `%s`\n"                      \
             "tokens: %s\n"                       \
             "expected: %s\n"                       \
             "------------",
             fp, line, src_code,
             lexed, expected
    );

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

int onk_assert_match(
    CuTest *tc,
    struct onk_tk_token_match_t *kit,
    struct onk_token_t *input,
    uint16_t ninput,
    uint16_t iter,
    char * filepath,
    uint16_t line
){
    char buf[512];

    char token2[128];
    char token1[128];

    const char *fmt_buf;

    fmt_buf = "failed size match. L%d (%s)";
    snprintf(buf, 512, fmt_buf, line, filepath);

    CuAssert(tc, buf, ninput != kit->narr);
    memset(buf, 0, 512);

    for(uint16_t i=0; ninput > i; i++)
    {
        switch(kit->arr[i].slot_type)
        {
            case onk_static_slot:
                fmt_buf =                                   \
                    "Token did not match STATIC match.\n "  \
                    "got: %s \n"                            \
                    "expected: %s \n"                       \
                    "failed L%d (%s)";

                snprintf(
                    buf, 512, fmt_buf,
                    onk_ptoken(input[i].type),
                    onk_ptoken(kit->arr[i].data.static_tok),
                    line, filepath
                );

                CuAssert(
                    tc,
                    buf,
                    kit->arr[i].data.static_tok != input[i].type
                );

                break;

            case onk_dynamic_slot:
                fmt_buf = "Token did not match DYNAMIC match (idx: %ud)\n" \
                    "got: %s \n"                                        \
                    "expect: %s \n"                                     \
                    "failed L%d (%s)";

                snprintf(
                    buf, 512, fmt_buf,
                    onk_ptoken(input[i].type),
                    onk_ptoken(kit->arr[i].data.static_tok),
                    line, filepath
                );

                CuAssert(
                    tc,
                    buf,
                    kit->arr[i].data.dyn_tok.arr[iter] != input[i].type
                );

                break;

            case onk_inspect_slot:
                fmt_buf = "Token did not match INSPECT match (mask: %ud)\n" \
                    "expected: %s\n"                                    \
                    "got: %s \n"                                        \
                    "failed L%d (%s)";

                onk_snprint_token(token1, 128, &input[i]);
                onk_snprint_token(token2, 128, &kit->arr[i].data.inspect.token);
                snprintf(buf, 512, fmt_buf, &kit->arr[i].data.inspect.flags, &token1, &token2, );

                CuAssert(
                    tc,
                    buf,
                    handle_inspect_slot(&kit->arr[i].data.inspect, &input[i]) == 0
                );

                break;

            default:
                fmt_buf = "Undefined Condition. Failed L%d (%s)";
                snprintf(buf, 512, fmt_buf, line, filepath);
                CuFail(tc, buf);
                return -1;
        }

        memset(buf, 0, 512);
    }
    return 0;
}


void onk_assert_tokenize(
    CuTest *tc,
    struct onk_tk_token_match_t *kit,
    struct onk_lexer_input_t *lexer_input,
    struct onk_lexer_output_t *lexer_output,
    uint16_t iter,
    char * fp,
    uint16_t line
){
    int ret;
    ret = onk_tokenize(lexer_input, lexer_output);

    // cuAssert(ret == 0)

    ret = onk_assert_match(
        tc,
        kit,
        lexer_output->tokens.base,
        lexer_input->tokens.len,
        iter,
        fp,
        line
    );

    // cuAssert(ret == 0)
}


void onk_assert_postfix(
    CuTest *tc,
    struct onk_tk_token_match_t *kit,
    struct onk_parser_input_t *input,
    struct onk_parser_output_t *output,
    uint16_t i,
    char * fp,
    uint16_t line
){
    int ret;

    ret = onk_parse(input, output);
    // cuAssert(ret == 0)
    ret = onk_assert_match(
        tc,
        kit,
        output->postfix.base,
        input->tokens.len,
        i,
        fp,
        line
    );

    // cuAssert(ret == 0)
}

void onk_assert_parse_stage(
    CuTest *tc,
    struct onk_tk_token_match_t *lexer,
    struct onk_tk_token_match_t *parser,
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

void build_test_mold_kit(
    struct onk_tk_token_match_t *parser,
    struct onk_tk_token_match_t *lexer)
{
    const enum onk_lexicon_t static_answ[] = { ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, 0 };
    const enum onk_lexicon_t dyn_answ_s1[] = { ONK_MUL_TOKEN, ONK_DIV_TOKEN, 0 };
    const enum onk_lexicon_t dyn_answ_s2[] = { ONK_ADD_TOKEN, ONK_SUB_TOKEN, 0 };

    enum onk_lexicon_t working_space = ONK_WORD_TOKEN;

    // mold lexer output
    tk_add_static(lexer, &working_space, 1);
    tk_add_dynamic(lexer, (enum onk_lexicon_t *)dyn_answ_s2, 2);
    tk_add_static(lexer, &working_space, 1);
    tk_add_dynamic(lexer, (enum onk_lexicon_t *)dyn_answ_s1, 2);

    // Mold Parser output
    tk_add_static(parser,  (enum onk_lexicon_t *)static_answ, 3);
    tk_add_dynamic(parser, (enum onk_lexicon_t *)dyn_answ_s1, 2);
    tk_add_dynamic(parser, (enum onk_lexicon_t *)dyn_answ_s2, 2);
}


void example(CuTest *tc)
{
    struct onk_tk_token_match_t parser, lexer;
    struct onk_vec_t tokens;

    struct onk_lexer_input_t lexer_input;
    struct onk_desc_token_t lex_buf[12];
    struct onk_desc_token_t parse_buf[12];

    const char * fmt_segs[][2] =                \
        {{"+", "-"},{"*", "/"}};
        /*s1p1  s1p2  s2p1 s2p2*/

    const char * fmt = "word %s word %s word";

    char buf[256];

    tk_init(&lexer, (struct onk_desc_token_t *)lex_buf, 12);
    tk_init(&parser, (struct onk_desc_token_t *)parse_buf, 12);
    onk_vec_init(&tokens, 32, sizeof(struct onk_token_t));

    build_test_mold_kit(&parser, &lexer);

    lexer_input.tokens = tokens;

    for(int i=0; 2 > i; i++)
    {
        snprintf(buf, 256, fmt, fmt_segs[i][0], fmt_segs[i][1]);
        lexer_input.src_code = buf;

        OnkAssertParseStage(tc, &lexer, &parser, i);

        onk_vec_clear(&tokens);
    }


    /* kt_run */
}


/*
    function determines if an expression is unbalanced.
    an expression can be unbalanced
    if there is a nonmatching `[` / `(` / `{` character
*/

/* void onk_assert_tokens( */
/*     CuTest *tc, */
/*     const char *source_code, */
/*     const char *file, */
/*     const char *msg, */
/*     const struct onk_token_t tokens[], */
/*     const enum onk_lexicon_t answer[] */
/* ){ */
/*     char uneql_msg[2048]; */
/*     char uneql_len_msg[2048]; */

/*     char got[512]; */
/*     char expected[512]; */

/*     uint16_t len=0; */
/*     for (;; len++) { */
/*         if (answer[len] == 0){ */
/*             break; */
/*         } */
/*     } */

/*     sprintf_token_slice(tokens, len, got, 512); */
/*     sprintf_lexicon_slice(answer, len, expected, 512); */
/*     sprintf(uneql_msg, FMT_STR, msg, expected, got, source_code); */
/*     sprintf(uneql_len_msg, "expected len: %ld <\n%s", len, uneql_msg); */

/* } */

/* void onk_assert_tokens_by_ref( */
/*     CuTest *tc, */
/*     const char *source_code, */
/*     const char *file, */
/*     const char *msg, */
/*     const struct onk_token_t *tokens[], */
/*     const enum onk_lexicon_t answer[] */
/* ){ */
/*     char uneql_msg[2048]; */
/*     char uneql_len_msg[2048]; */

/*     char got[512]; */
/*     char expected[512]; */

/*     memset(got, 0, sizeof(char[512])); */
/*     memset(expected, 0, sizeof(char[512])); */
/*     memset(uneql_len_msg, 0, sizeof(char[2048])); */
/*     memset(uneql_msg, 0, sizeof(char[2048])); */

/*     usize len=0; */
/*     for (;; len++) { */
/*         if (answer[len] == 0) */
/*             break; */
/*     } */
/*     if (len == 0) */
/*         return; */

/*     sprintf_token_slice_by_ref(tokens, len, got, 512); */
/*     sprintf_lexicon_slice(answer, len, expected, 512); */
/*     sprintf(uneql_msg, FMT_STR, msg, expected, got, source_code); */
/*     sprintf(uneql_len_msg, "%s: expected len: %ld <\n%s", msg, len, uneql_msg); */
/* } */
