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
  onk_known_slot,
  onk_answer_slot
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
struct token_desc_t
{
    enum descriptor_ty type;
    enum onk_lexicon_t tok;
};


/*
 * formats string, tokenizes, parses,
 *
 * checks output against --
 * */
struct token_mold_t
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
    onk_stage_ident_ast
};


union FailState {
    struct {
        struct onk_lexer_output_t output;
        struct onk_lexer_input_t input;
    } lexer;

    struct {
        struct onk_parser_input_t input;
        struct onk_parser_output_t output;
    } parser;

    struct {
        struct token_mold_t mold;
        uint16_t failed_at_mold_idx;
    } testkit;
};


struct onk_testkit_result_t {
    enum stage_failure result;

};

struct test_unit {
    struct token_mold_t * molds;
    //struct onk_vec_t * working_buf;
    // { "", .., 0}
    const char **source;

};

struct testkit {

};


uint16_t _add_descriptor(
    struct token_mold_t * mold,
    enum onk_lexicon_t tok,
    enum descriptor_ty type
){
    struct token_desc_t dtok;
    uint16_t idx;

    idx = mold->narr;
    dtok.tok = tok;
    dtok.type = type;

    mold->arr[idx] = dtok;
    mold->narr += 1;
    return idx;
}

/*
 *
*/
int8_t onk_mold_add_known(
    struct token_mold_t * mold,
    enum onk_lexicon_t tok)
{
    if(mold->sarr > mold->narr)
        return -1;

    _add_descriptor(mold, tok, onk_known_slot);
    return 1;
}

int onk_mold_add_unknown(
    struct token_mold_t * mold,
    enum onk_lexicon_t *answers,
    uint16_t nanswers)
{
    if(mold->sarr > mold->narr)
        return -1;

    _add_descriptor(mold, 0, onk_answer_slot);

    mold->answers[mold->nanswers] = answers;
    mold->ianswers[mold->nanswers] = nanswers;
    mold->nanswers += 1;

    return 1;
}


const struct onk_token_t * _get_token(struct onk_parser_output_t *pout, uint16_t idx)
{
    return &((struct onk_token_t *)pout->postfix.base)[idx];
}



enum stage_failure _handle_generic_mold(
    struct token_mold_t *mold,
    struct onk_parser_output_t pout,
    uint16_t itoken,
    uint16_t nfmt)
{
    bool generic_flag = false;

    for(uint16_t z = 0; mold->ianswers[nfmt] > z; z++)
    {
        if (mold->answers[nfmt][z] == _get_token(&pout, itoken)->type)
        {
            generic_flag = true;
            break;
        }
    }

    if (!generic_flag)
        return onk_stage_ident_testfail;

    return onk_stage_OK;
}

enum stage_failure onk_apply_mold(
    struct token_mold_t * mold,
    struct onk_vec_t tokens,
    const char * fmt,
    const char **subs
){
    char buf[128];

    struct token_desc_t desc_tok;
    struct onk_token_t working_tok;

    struct onk_lexer_output_t l_out;
    struct onk_lexer_input_t l_in;
    struct onk_parser_input_t p_in;
    struct onk_parser_output_t p_out;

    uint16_t ctr = 0;
    bool generic_flag = 0;

    l_in.tokens = tokens;

    for (int i=0 ;; i++)
    {
        if (subs[i] == 0)
            break;

        /* format string to tokenize
         *
         * eg: 10 %s 10
         *         +
         *         -
         *         *
         *
         *
         * */
        snprintf((char *)&buf, 128, fmt, subs[i]);

        l_in.src_code = (const char *)&buf;

        if(onk_tokenize(&l_in, &l_out))
            return onk_stage_ident_lex;

        onk_parser_input_from_lexer_output(&l_out, &p_in, false);

        if(onk_parse(&p_in, &p_out) != 0)
          return onk_stage_ident_parse;


        for(uint16_t i=0; mold->narr > i; i++)
        {
            desc_tok = mold->arr[i];

            if (desc_tok.type == onk_known_slot
              && _get_token(&p_out, i)->type != desc_tok.tok)
              return onk_stage_ident_testfail;

            else if (desc_tok.type == onk_answer_slot)
                _handle_generic_mold(
                    mold,
                    &p_out,
                    mold->ianswers[ctr],
                    ctr);


            else
              return onk_stage_ident_udef;


        }

        onk_vec_clear(&tokens);
    }
}




int test_mold() {

    struct Kit kit;
    enum onk_lexicon_t knowns[] = { ONK_WORD_TOKEN, ONK_WORD_TOKEN, 0 };
    enum onk_lexicon_t answers[] = { ONK_ADD_TOKEN, ONK_SUB_TOKEN, 0 };
    enum onk_lexicon_t knowns_p2 = ONK_WORD_TOKEN;

    char buf[256];

    const char * fmt_segs[][2] = \
        {{"+", "-"}, {"*", "/"}};
        /*s1p1  s1p2  s2p1 s2p2*/

    const char * fmt = "word %s word %s word";


    build_kt_add_known(&kit, &knowns, 2);
    build_kt_add_answer(&kit, &answers, &fmt_segs);
    build_kit_add_known(&kit, &knowns_p2, 1);


    const char * fmt = "word %s word %s word";
    for(int i=0; 2 > i; i++)
    {
        snprintf(&buf, fmt, 256, fmt_segs[i][0], fmt_segs[i][1]);


    }


    kt_run

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
