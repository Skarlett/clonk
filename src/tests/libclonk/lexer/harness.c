#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "onkstd/llist.h"
#include "onkstd/merge_sort.h"
#include "libtest/CuTest.h"
#include "libtest/assert.h"
#include "lexer.h"

enum _onk_test_expected_output {
  _onk_test_exp_answers,
  _onk_test_exp_generated
};

const char * error_missing_answers(
    enum onk_lexicon_t *answers,
    uint16_t *missing,
    uint16_t nmissing,
    enum _onk_test_expected_output mode
){
    const char *prefix="\n------------\nMissing from generated:  \n";
    char * message = malloc(2048);
    uint16_t message_sz = 2048;
    uint16_t message_len = 0;
    char tname_buf[64] = {0};
    uint16_t tname_len = 0;

    strcat(message, prefix);
    message_len = strlen(prefix) - 1;

    for(uint16_t k=0; nmissing > k; k++)
    {
        memset(tname_buf, 0, 64);

        if (mode == _onk_test_exp_generated)
          tname_len = snprintf(tname_buf, 64, "[%s(answer index: %u)]\n", onk_ptoken(answers[missing[k]]), missing[k]);
        else if(mode == _onk_test_exp_answers)
          tname_len = snprintf(tname_buf, 64, "[%s(answer index: %u)]\n", onk_ptoken(missing[k]), k);
        else
          assert(false);

        assert(tname_len);
        if(message_len + tname_len >= message_sz)
        {
            message = realloc(message, message_sz * 2);
            assert(message);
            message_sz *= 2;
        }
        strncat(message, tname_buf, message_sz - message_len);
        message_len += tname_len;
    };
    strncat(message, "\n------------", message_sz - message_len);
    return message;
}


#define HARNESS_SLOTS PH_ONK_TOKEN_END+1
void _lex_range_harness(
    CuTest *tc,
    const char *file,
    uint16_t line,
    handler handler,
    enum onk_lexicon_t *answers,
    uint16_t answers_len
){
    enum onk_lexicon_t tok = 0;
    enum onk_lexicon_t generated[HARNESS_SLOTS];
    enum onk_lexicon_t clean_generated[HARNESS_SLOTS];
    struct onk_llist_t popmap[HARNESS_SLOTS];

    struct onk_llist_t *popmap_root=0, *popmap_step=0;
    struct onk_llist_rm_t rmret;

    uint16_t missing[HARNESS_SLOTS];
    uint16_t missing_idx = 0, popmap_i=0;
    int16_t contains_idx = 0, gen_idx=0;

    CuAssert(tc, "harness called with null pointer, or empty answers", answers_len != 0);
    CuAssert(tc, "harness called with null pointer, or empty answers", answers != 0);

    onk_bubblesort_lexarr(answers, answers_len);

    /* generate all tokens  */
    for(tok = PH_ONK_TOKEN_START; PH_ONK_TOKEN_END > tok; tok++)
    {
        if(handler(tok))
        {
            popmap[gen_idx].i = (uint16_t)tok;
            generated[gen_idx] = tok;
            gen_idx += 1;
        }
    }

    /* add missing tokens (from answers) to the missing buffer */
    for (uint16_t i=0; answers_len > i; i++)
    {
        contains_idx = onk_lexarr_contains(answers[i], generated, gen_idx);
        if (contains_idx == -1)
            missing[missing_idx++] = i;
    }

    /* print missing tokens which were not found in the answer list */
    if(missing_idx)
        CuFail_Line(
            tc,
            file,
            line,
            NULL,
            error_missing_answers(answers, missing, missing_idx, _onk_test_exp_generated)
        );


    /* initialize linked list */
    for (uint16_t j=0; gen_idx > j; j++)
    {
        popmap[j].next = 0;
        if (gen_idx - 1 > j)
            popmap[j].next = &popmap[j+1];
    }

    /* remove all illegal tokens from handler-generation */
    popmap_root = &popmap[0];
    for(uint16_t k=0; ILLEGAL_TOKENS_LEN > k; k++)
    {
        rmret = onk_llist_rm(popmap_root, ILLEGAL_TOKENS[k]);
        popmap_root = rmret.root;
    }

    /* flatten linked list into an array */
    popmap_step = popmap_root;
    while(popmap_step)
    {
        clean_generated[popmap_i++] = popmap_step->i;

        /* extra generated tokens not included in the answer list will be marked as missing */
        if (onk_lexarr_contains(popmap_step->i, answers, (int16_t)answers_len) == -1)
            missing[missing_idx++] = popmap_step->i;

        if (popmap_step->next)
            popmap_step = popmap_step->next;

        else break;
    }

    /* assert the answer's length is the same as the generated */
    if(answers_len != popmap_i)
        CuFail_Line(tc, file, line, "len equality check failed",
                    error_missing_answers(answers, missing, missing_idx, _onk_test_exp_answers));

    /* memcmp the flattened & sorted list */
    if(memcmp(answers, clean_generated, sizeof(enum onk_lexicon_t) * answers_len) != 0)
        CuFail_Line(tc, file, line, "memcmp failed",
                    error_missing_answers(answers, missing, missing_idx, _onk_test_exp_answers));
}


/******************
* test foundation *
******************/
void __test__illegal_tokens_length(CuTest *tc) {

    char *buf = malloc(256);
    uint8_t i=0;

    for(i=0; UINT8_MAX > i; i++)
    {
        if(ILLEGAL_TOKENS[i] == 0 && ILLEGAL_TOKENS_LEN == i)
          return;
    }
    snprintf(buf, 256, "the amount of illegal tokens got <%u>"    \
            "does not match ILLEGAL_TOKEN_LEN <%u>", i, ILLEGAL_TOKENS_LEN);

    CuFail(tc, buf);
}

/****************
* test bad hook *
****************/
bool bad_hook(enum onk_lexicon_t _)
{ return false; }

/***************
* test results *
***************/
bool test_handle_succeed(enum onk_lexicon_t tok)
{
    return tok == ONK_TRUE_TOKEN
        || tok == ONK_FALSE_TOKEN
        || tok == ONK_NULL_TOKEN;
}

/********************
* use lexer harness *
*********************/

void __test__is_tok_unary_operator(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_NOT_TOKEN, ONK_TILDE_TOKEN, 0
    };

    LexRangeHarness(tc, onk_is_tok_unary_operator, answers, 2);
}

void __test__is_tok_delimiter(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_SEMICOLON_TOKEN, ONK_COMMA_TOKEN,
      ONK_COLON_TOKEN,
      0
    };

    LexRangeHarness(tc, onk_is_tok_delimiter, answers, 3);
}

void __test__is_tok_brace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BRACE_OPEN_TOKEN,
      ONK_BRACE_CLOSE_TOKEN,

      ONK_BRACKET_OPEN_TOKEN,
      ONK_BRACKET_CLOSE_TOKEN,

      ONK_PARAM_CLOSE_TOKEN,
      ONK_PARAM_OPEN_TOKEN,

      ONK_HASHMAP_LITERAL_START_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_brace,
        answers, 7
    );
}

void __test__is_tok_open_brace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BRACE_OPEN_TOKEN,
      ONK_BRACKET_OPEN_TOKEN,
      ONK_PARAM_OPEN_TOKEN,
      ONK_HASHMAP_LITERAL_START_TOKEN,
      0
    };

    LexRangeHarness(tc, onk_is_tok_open_brace, answers, 4);
}

void __test__is_tok_close_brace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BRACE_CLOSE_TOKEN,
      ONK_BRACKET_CLOSE_TOKEN,
      ONK_PARAM_CLOSE_TOKEN,
    };

    LexRangeHarness(
        tc,
        onk_is_tok_close_brace,
        answers, 3
    );
}

void __test__is_tok_whitespace(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_NEWLINE_TOKEN,
      ONK_WHITESPACE_TOKEN,
    };

    LexRangeHarness(
        tc,
        onk_is_tok_whitespace,
        answers,
        2
    );
}

void __test__is_tok_block_keyword(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_IF_TOKEN, ONK_ELSE_TOKEN, ONK_DEF_TOKEN,
      ONK_FOR_TOKEN, ONK_WHILE_TOKEN, ONK_FROM_TOKEN,
      ONK_BREAK_TOKEN, ONK_CONTINUE_TOKEN, ONK_IMPL_TOKEN,
      ONK_IMPORT_TOKEN, ONK_RETURN_TOKEN, ONK_STRUCT_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_block_keyword,
        answers,
        12
    );
}

void __test__is_tok_keyword(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_IF_TOKEN, ONK_ELSE_TOKEN, ONK_DEF_TOKEN,
      ONK_FOR_TOKEN, ONK_WHILE_TOKEN, ONK_FROM_TOKEN,
      ONK_BREAK_TOKEN, ONK_CONTINUE_TOKEN, ONK_IMPL_TOKEN,
      ONK_IMPORT_TOKEN, ONK_RETURN_TOKEN, ONK_STRUCT_TOKEN,

      ONK_IN_TOKEN, ONK_TRUE_TOKEN, ONK_FALSE_TOKEN, ONK_NULL_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_keyword,
        answers,
        16
    );
}

void __test__is_tok_unit(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_TRUE_TOKEN,
      ONK_FALSE_TOKEN,
      ONK_NULL_TOKEN,
      ONK_INTEGER_TOKEN,
      ONK_WORD_TOKEN,
      ONK_STRING_LITERAL_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_unit,
        answers,
        6
    );
}

void __test__is_tok_loopctl(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_CONTINUE_TOKEN,
      ONK_BREAK_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_loopctlkw,
        answers, 2
    );
}

void __test__is_tok_binop(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BIT_OR_EQL,
      ONK_BIT_AND_EQL,
      ONK_BIT_NOT_EQL,
      ONK_MINUS_EQL_TOKEN,
      ONK_PLUSEQ_TOKEN,

      ONK_EQUAL_TOKEN,
      ONK_ADD_TOKEN,
      ONK_SUB_TOKEN,
      ONK_MUL_TOKEN,
      ONK_DIV_TOKEN,

      ONK_POW_TOKEN,
      ONK_DOT_TOKEN,
      ONK_GT_TOKEN,
      ONK_LT_TOKEN,
      ONK_GT_EQL_TOKEN,

      ONK_MOD_TOKEN,
      ONK_AND_TOKEN,
      ONK_SHR_TOKEN,
      ONK_SHL_TOKEN,
      ONK_OR_TOKEN,

      ONK_AMPER_TOKEN,
      ONK_LT_EQL_TOKEN,
      ONK_ISEQL_TOKEN,
      ONK_NOT_EQL_TOKEN,
      ONK_PIPE_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_binop,
        answers,
        25
    );
}

void __test__is_tok_asn_op(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      ONK_BIT_OR_EQL,
      ONK_BIT_AND_EQL,
      ONK_BIT_NOT_EQL,
      ONK_MINUS_EQL_TOKEN,
      ONK_PLUSEQ_TOKEN,
      ONK_EQUAL_TOKEN,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_asn_operator,
        answers,
        6
    );
}

void __test__is_tok_group_modifier(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      onk_apply_op_token,
      onk_ifcond_op_token,
      onk_ifbody_op_token,
      onk_defsig_op_token,
      onk_defbody_op_token,
      onk_for_args_op_token,
      onk_for_body_op_token,
      onk_struct_init_op_token,
      onk_idx_op_token,
      onk_while_body_op_token,
      onk_while_cond_op_token,
      0,
    };

    LexRangeHarness(
        tc,
        onk_is_tok_group_modifier,
        answers,
        11
   );
}

void __test__is_tok_group_ident(CuTest *tc)
{
    enum onk_lexicon_t answers[] = {
      onk_map_group_token,
      onk_code_group_token,
      onk_list_group_token,
      onk_struct_group_token,
      onk_idx_group_token,
      onk_tuple_group_token,
      0
    };

    LexRangeHarness(
        tc,
        onk_is_tok_group,
        answers,
        6
   );
}

void __test__all_tokens_have_ptoken_impl(CuTest *tc)
{
    const char * no_name = onk_ptoken(PH_ONK_TOKEN_START);
    const char * name = 0;
    char msg[256];

    for(enum onk_lexicon_t i=PH_ONK_TOKEN_START; PH_ONK_TOKEN_END > i; i++)
    {
        /* roll over illegal tokens */
        if (onk_lexarr_contains(i, (enum onk_lexicon_t *)ILLEGAL_TOKENS, ILLEGAL_TOKENS_LEN) > -1)
            continue;

        name = onk_ptoken(i);
        if(name == no_name)
        {
            snprintf(msg, 256, "failed on idx %d [got: %s]", i, name);
            CuFail(tc, msg);
        }
    }
}

CuSuite* LexerHarnessLogicTests(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, __test__illegal_tokens_length);
    SUITE_ADD_TEST(suite, __test__all_tokens_have_ptoken_impl);

    SUITE_ADD_TEST(suite, __test__is_tok_close_brace);
    SUITE_ADD_TEST(suite, __test__is_tok_open_brace);
    SUITE_ADD_TEST(suite, __test__is_tok_brace);

    SUITE_ADD_TEST(suite, __test__is_tok_unary_operator);
    SUITE_ADD_TEST(suite, __test__is_tok_binop);

    SUITE_ADD_TEST(suite, __test__is_tok_asn_op);
    SUITE_ADD_TEST(suite, __test__is_tok_keyword);
    SUITE_ADD_TEST(suite, __test__is_tok_block_keyword);
    SUITE_ADD_TEST(suite, __test__is_tok_loopctl);
    SUITE_ADD_TEST(suite, __test__is_tok_delimiter);
    SUITE_ADD_TEST(suite, __test__is_tok_group_ident);
    SUITE_ADD_TEST(suite, __test__is_tok_group_modifier);

    return suite;
}
