#include "libtest/CuTest.h"
#include "lexer.h"
#include "parser.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define FMT_STR "%s\nexpected: \n%s\ngot: \n%s\nsrc: \"%s\""


/**
 * Checks if token stream is balanced.
 * To be balanced is every brace opening,
 * having a pairing brace closing token
 * following it eventually.
 * The follow are examples:
 *
 *    a + b + (2 + 5)  Is balanced
 *   (a + b            Is unbalanced.
 *
 * @param tokens array of tokens
 * @param ntokens amount of tokens to read
 *
 * @return bool
 */
//bool is_balanced(struct onk_token_t tokens[], uint16_t ntokens);

enum descriptor_ty
{
  onk_static_slot,
  onk_dynamic_slot,
  onk_inspect_slot
};


/*
 *
 *
 *                              a1
 * [ Dtoken<WORD>, Dtoken<INT>, Dtoken< ? >, 0 ]
 *                               001   Add
 *                               010   Sub
 *                               100   Mul
 *
 *
 *
 * */

struct dyn_descriptor {
    enum onk_lexicon_t *arr;
    uint16_t narr;
};

struct inspect_descriptor {
    enum onk_lexicon_t group_type;
    uint16_t start;
    uint16_t end;
    uint16_t seq;
};

struct token_desc_t {
    enum descriptor_ty slot_type;

    union {
        enum onk_lexicon_t static_tok;
        struct inspect_descriptor inspect;
        struct dyn_descriptor dyn_tok;
    } data;
};

/* struct token_desc_t */
/* { */
/*     enum descriptor_ty type; */
/*     enum onk_lexicon_t tok; */
/* }; */

/*
 * formats string, tokenizes, parses,
 *
 * checks output against --
 * */
struct onk_tk_token_match_t
{
    struct token_desc_t * arr;

    /*number*/
    uint16_t narr;

    /*size*/
    uint16_t sarr;

    /* /\* array idx answer *\/ */
    const enum onk_lexicon_t ** answers;

    /* /\* [nslice, nslice]  *\/ */
    uint16_t * ianswers;

    uint16_t nanswers;
};


enum stage_failure
{
    onk_stage_OK,
    onk_stage_ident_testfail,
    onk_stage_ident_udef,
    onk_stage_ident_lex,
    onk_stage_ident_parse,
    onk_stage_ident_validate,
};

struct onk_testkit_result_t {
    enum stage_failure result;

};

/*
 * copies (`enum onk_lexicon_t *tok`) each item into
 * `onk_tk_token_match_t` as a static option.
*/
int8_t tk_add_static(
    struct onk_tk_token_match_t * mold,
    enum onk_lexicon_t *tok,
    uint16_t nitems)
{
    struct token_desc_t dtok;

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

int8_t tk_add_grp(
    struct onk_tk_token_match_t *kit,
    enum onk_lexicon_t *tok,
    uint16_t items
)
{


    
        _add_descriptor(mold, tok[i], onk_static_slot);
}

int8_t tk_add_static_repeating(
    struct onk_tk_token_match_t *kit,
    enum onk_lexicon_t tok,
    uint16_t ntimes
)
{
    bool overflow = false;
    struct token_desc_t descriptor;
    struct token_desc_t *ptr;

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

int tk_add_dynamic(
    struct onk_tk_token_match_t * kit,
    const enum onk_lexicon_t *answers,
    uint16_t nanswers)
{
    if(kit->sarr > kit->narr)
        return -1;

    kit->answers[kit->nanswers] = answers;
    kit->ianswers[kit->nanswers] = nanswers;
    kit->nanswers += 1;
    return 1;
}

int kt_test(
    struct onk_tk_token_match_t *kit,
    struct onk_token_t *input,
    uint16_t ninput,
    uint16_t seg_i
){
    uint16_t fmt_i = 0;

    //todo assert
    if(ninput != kit->narr)
      return -1;

    for(uint16_t i=0; ninput > i; i++)
    {
        switch(kit->arr[i].type) {
            case onk_static_slot:
                if(kit->arr[i].tok != input[i].type)
                  return -1;
                break;

            case onk_dynamic_slot:
                if(kit->answers[fmt_i][seg_i] != input[i].type)
                    return -1;

                fmt_i += 1;
                break;

            case onk_group_slot:


            default:
                return -1;
        }
    }

    return 0;

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


void kt_tokenize_test(
    struct onk_tk_token_match_t *kit,
    struct onk_lexer_input_t *lexer_input,
    struct onk_lexer_output_t *lexer_output,
    uint16_t i
){
    int ret;
    ret = onk_tokenize(lexer_input, lexer_output);

    // cuAssert(ret == 0)

    ret = kt_test(
        kit,
        lexer_output->tokens.base,
        lexer_input->tokens.len,
        i
    );

    // cuAssert(ret == 0)
}


void kt_parse_test(
    struct onk_tk_token_match_t *kit,
    struct onk_parser_input_t *input,
    struct onk_parser_output_t *output,
    uint16_t i
){
    int ret;

    ret = onk_parse(input, output);
    // cuAssert(ret == 0)
    ret = kt_test(
        kit,
        output->postfix.base,
        input->tokens.len,
        i
    );

    // cuAssert(ret == 0)
}

int test_mold()
{
    struct onk_tk_token_match_t parser, lexer;
    struct onk_vec_t tokens;

    struct onk_lexer_input_t lexer_input;
    struct onk_lexer_output_t lexer_output;
    struct onk_parser_input_t parser_input;
    struct onk_parser_output_t parser_output;

    const char * fmt_segs[][2] =                \
        {{"+", "-"},{"*", "/"}};
        /*s1p1  s1p2  s2p1 s2p2*/

    const char * fmt = "word %s word %s word";

    char buf[256];
    int kt_ret;
    int ret;

    build_test_kit_mold(&parser, &lexer);

    lexer_input.tokens = tokens;
    onk_vec_init(&tokens, 256, sizeof(struct onk_token_t));

    for(int i=0; 2 > i; i++)
    {
        snprintf(buf, 256, fmt, fmt_segs[i][0], fmt_segs[i][1]);
        lexer_input.src_code = buf;

        ret = onk_tokenize(&lexer_input, &lexer_output);

        kt_ret = kt_test(
            &lexer,
            tokens.base,
            tokens.len,
            i
        );

        if(kt_ret == -1)
            return -1;

        onk_parser_input_from_lexer_output(
            &lexer_output, &parser_input, false
        );

        ret = onk_parse(&parser_input, &parser_output);

        kt_ret = kt_test(
            &parser,
            tokens.base,
            tokens.len,
            i
        );

        if(kt_ret == -1)
            return -1;

        onk_vec_clear(&tokens);
    }


    /* kt_run */

}

/**
 * Checks if token stream is balanced by reference. see (src/parser/lexer/helpers.h#is_balance)
 *
 * @param tokens array of referenced tokens
 * @param ntokens amount of tokens to read
 *
 * @return bool
 */
//bool is_balanced_by_ref(struct onk_token_t *tokens[], uint16_t ntokens);

void onk_assert_parse(
    CuTest *tc,
    const char ** source_code,
    const char * file,
    const char * failure_mesg,
    const enum onk_lexicon_t *answers
)
{



}

/*
    function determines if an expression is unbalanced.
    an expression can be unbalanced
    if there is a nonmatching `[` / `(` / `{` character
*/

void onk_assert_tokens(
    CuTest *tc,
    const char *source_code,
    const char *file,
    const char *msg,
    const struct onk_token_t tokens[],
    const enum onk_lexicon_t answer[]
){
    char uneql_msg[2048];
    char uneql_len_msg[2048];
    
    char got[512];
    char expected[512];
    
    uint16_t len=0;
    for (;; len++) {
        if (answer[len] == 0){
            break;
        }
    }

    sprintf_token_slice(tokens, len, got, 512);
    sprintf_lexicon_slice(answer, len, expected, 512);
    sprintf(uneql_msg, FMT_STR, msg, expected, got, source_code);
    sprintf(uneql_len_msg, "expected len: %ld <\n%s", len, uneql_msg);

}

void onk_assert_tokens_by_ref(
    CuTest *tc,
    const char *source_code,
    const char *file,
    const char *msg,
    const struct onk_token_t *tokens[],
    const enum onk_lexicon_t answer[]
){
    char uneql_msg[2048];
    char uneql_len_msg[2048];
    
    char got[512];
    char expected[512];
    
    memset(got, 0, sizeof(char[512]));
    memset(expected, 0, sizeof(char[512]));
    memset(uneql_len_msg, 0, sizeof(char[2048]));
    memset(uneql_msg, 0, sizeof(char[2048]));

    usize len=0;
    for (;; len++) {
        if (answer[len] == 0)
            break;
    }
    if (len == 0)
        return;

    sprintf_token_slice_by_ref(tokens, len, got, 512);
    sprintf_lexicon_slice(answer, len, expected, 512);
    sprintf(uneql_msg, FMT_STR, msg, expected, got, source_code);
    sprintf(uneql_len_msg, "%s: expected len: %ld <\n%s", msg, len, uneql_msg);
}
