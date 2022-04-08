#include <string.h>
#include "onkstd/vec.h"
#include "lexer.h"
#include "parser.h"

#include "libtest/tokens.h"
#include "libtest/CuTest.h"

/*
 * copies (`enum onk_lexicon_t *tok`) each item into
 * `onk_test_mask_t` as a static option.
*/
int8_t onk_desc_add_static_slot(
    struct onk_test_mask_t * mold,
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
    struct onk_test_mask_t *kit,
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
    struct onk_test_mask_t * kit,
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
    struct onk_test_mask_t *kit,
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
    struct onk_test_mask_t *kit,
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
    struct onk_test_mask_t *kit,
    struct onk_desc_token_t *buffer,
    uint16_t buffer_sz
)
{
    kit->arr = buffer;
    kit->narr = buffer_sz;
}


int16_t print_expect_line(
    char * buf,
    uint16_t nbuf,
    char * src_code,
    struct onk_token_t *lexed,
    enum onk_lexicon_t *expected,
    char * msg,
    uint16_t mismatched_idx,
    char * fp,
    uint16_t line
){
    char lexed_token[ONK_TOK_CHAR_SIZE];
    char expected_token[ONK_TOK_CHAR_SIZE];

    uint16_t spaces = 0;
    uint16_t underline = 0;
    uint16_t nwrote;

    char * tail;

    onk_snprint_token_type(
        lexed_token, ONK_TOK_CHAR_SIZE,
        lexed[mismatched_idx].type
    );

    onk_snprint_token_type(
        expected_token, ONK_TOK_CHAR_SIZE,
        expected[mismatched_idx]
    );

    for(uint16_t i = 0; mismatched_idx > i; i++)
        spaces += onk_token_len(&lexed[i]);

    underline = onk_token_len(&lexed[mismatched_idx]);

    nwrote = snprintf(
        buf, nbuf,
        "[%s] failed at: L%u (expected `%s` got `%s`)\n"    \
        "%s\n"                                              \
        "src: `%s`\n",
        fp, line, expected_token, lexed_token, msg, src_code
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

int16_t print_lex_type_mismatch(
    char * buf,
    uint16_t nbuf,
    char * src_code,
    struct onk_token_t *lexed,
    uint16_t nlexed,
    enum onk_lexicon_t *expected,
    uint16_t nexpected,
    uint16_t mismatched_idx,
    char * fp,
    uint16_t line
){
    uint16_t spaces = 0;
    uint16_t underline = 0;
    int16_t nwrote;

    uint16_t lex_sz =                                               \
        (onk_str_len_token_arr(lexed, nlexed) + 1) * sizeof(char);

    uint16_t expect_sz =                                                \
        (onk_str_len_lexicon_arr(expected, nexpected) + 1) * sizeof(char);

    char * lexicon_buf = malloc(lex_sz);
    char * expected_buf = malloc(expect_sz);
    char * tail;

    nwrote = print_expect_line(
        buf,
        nbuf,
        src_code,
        lexed,
        expected,
        mismatched_idx,
        fp,
        line
    );

    if(nwrote == -1)
        return -1;

    for(uint16_t i = 0; mismatched_idx > i; i++)
        spaces += (strlen(onk_ptoken(expected[i])) || 1);

    underline = strlen(onk_ptoken(expected[mismatched_idx]));

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


int16_t kit_as_lex_arr(
    enum onk_lexicon_t *arr,
    uint16_t arr_sz,
    struct onk_test_mask_t *kit,
    uint16_t fmt_i,
    uint16_t *inspect, // [idx, 4, ..]
    uint16_t inspect_sz
){
    uint16_t i;

    uint16_t ninspect;

    for(i=0; kit->narr > i; i++) {
        if(i >= arr_sz)
            return -1;

        switch(kit->arr[i].slot_type)
        {
            case onk_static_slot:
                arr[i] = kit->arr[i].data.static_tok;
                break;

            case onk_dynamic_slot:
                arr[i] = kit->arr[i].data.dyn_tok.arr[fmt_i];
                break;

            case onk_inspect_slot:
                if(ninspect >= inspect_sz)
                    return -1;

                arr[i] = kit->arr[i].data.inspect.token.type;
                inspect[ninspect] = i;
                break;

            default:
                return -1;
        }

    }

    return i + 1;
}


int onk_assert_match(
    CuTest *tc,
    struct onk_test_mask_t *kit,
    struct onk_token_t *input,
    uint16_t ninput,
    char * src_code,
    uint16_t iter,
    char * filepath,
    uint16_t line
){
    char buf[512];

    char token2[ONK_TOK_CHAR_SIZE];
    char token1[ONK_TOK_CHAR_SIZE];

    const char *fmt_buf;

    fmt_buf = "failed size match. L%d (%s)";
    snprintf(buf, 512, fmt_buf, line, filepath);

    CuAssert(tc, buf, ninput != kit->narr);
    memset(buf, 0, 512);


    kit_as_lex_arr()

    memcmp();

    for(uint16_t i=0; ninput > i; i++)
    {
        print_lex_type_mismatch(
            buf, 512, src_code,
            input, ninput, expected,
            nexpected, i, fp, line
        );

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
    struct onk_test_mask_t *kit,
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
    struct onk_test_mask_t *kit,
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

void build_test_mold_kit(
    struct onk_test_mask_t *parser,
    struct onk_test_mask_t *lexer)
{
    const enum onk_lexicon_t static_answ[] = { ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, 0 };
    const enum onk_lexicon_t dyn_answ_s1[] = { ONK_MUL_TOKEN, ONK_DIV_TOKEN, 0 };
    const enum onk_lexicon_t dyn_answ_s2[] = { ONK_ADD_TOKEN, ONK_SUB_TOKEN, 0 };

    enum onk_lexicon_t working_space = ONK_WORD_TOKEN;

    // mold lexer output
    onk_desc_add_static_slot(lexer, &working_space, 1);
    onk_desc_add_dynamic_slot(lexer, (enum onk_lexicon_t *)dyn_answ_s2, 2);
    onk_desc_add_static_slot(lexer, &working_space, 1);
    onk_desc_add_dynamic_slot(lexer, (enum onk_lexicon_t *)dyn_answ_s1, 2);

    // Mold Parser output
    onk_desc_add_static_slot(parser,  (enum onk_lexicon_t *)static_answ, 3);
    onk_desc_add_dynamic_slot(parser, (enum onk_lexicon_t *)dyn_answ_s1, 2);
    onk_desc_add_dynamic_slot(parser, (enum onk_lexicon_t *)dyn_answ_s2, 2);
}


void example(CuTest *tc)
{
    struct onk_test_mask_t parser, lexer;
    struct onk_vec_t tokens;

    struct onk_lexer_input_t lexer_input;
    struct onk_desc_token_t lex_buf[12];
    struct onk_desc_token_t parse_buf[12];

    const char * fmt_segs[][2] =            \
        {{"+", "-"},{"*", "/"}};
        /*s1p1  s1p2  s2p1 s2p2*/

    const char * fmt = "word %s word %s word";

    char buf[256];

    onk_match_token_init(&lexer, (struct onk_desc_token_t *)lex_buf, 12);
    onk_match_token_init(&parser, (struct onk_desc_token_t *)parse_buf, 12);
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
}
