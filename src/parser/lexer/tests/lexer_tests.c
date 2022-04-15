#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "libtest/CuTest.h"
#include "libtest/masking.h"
#include "lexer.h"


void __test__basic_perthensis(CuTest* tc)
{
    static uint16_t tokens_sz[] = {5, 7, 7, 9, 9, 11, 13};
    uint16_t ntokens=0;

    struct onk_token_t tokens[16];
    char msg[1028];
    
    char got[512];
    char expected[512];
    char token_buf[24];

    int offset_got = 0;
    int offset_expected = 0;
    int tmp = 0;

    const char * src_code[] =  {
        "(1 + 2)",        // 5
        "(1 + 2) + 3",    // 7
        "1 + (2 + 3)",    // 7
        "(1 + (2 + 3))",        // 9
        "((1 + 2) + 3)",         // 9,
        "(1 + 2) + (3 + 4)",     // 10,
        "((1 + 2) + (3 + 4))",   // 12,
        0
    };
    
    static enum onk_lexicon_t check_list[][16] = {
        {ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_ADD_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_ADD_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        0
    };

    for (uint16_t i=0 ;; i++) {
        if (check_list[i][0] == 0 || src_code[i] == 0)
            break;
        ntokens = 0;
        CuAssertTrue(tc, onk_tokenize(src_code[i], tokens, &ntokens, 16, false, NULL) == 0);
        CuAssertTrue(tc, ntokens == tokens_sz[i]);


        sprintf(msg, "failed on idx [%d] ", i);
        onk_assert_tokens(tc, src_code[i], msg, tokens, check_list[i]);
    }
}

void __test__collapse_integer(CuTest* tc)
{ 
    struct onk_token_t tokens[16];
    uint16_t i=0;

    CuAssertTrue(tc, onk_tokenize("1234", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == ONK_INTEGER_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 3);
    i=0;

    CuAssertTrue(tc, onk_tokenize("1", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == ONK_INTEGER_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 0);
    i=0;

    CuAssertTrue(tc, onk_tokenize(" 1 ", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == ONK_INTEGER_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 1);
    CuAssertTrue(tc, tokens[0].end == 1);
}

void __test__destroy_whitespace(CuTest* tc)
{
    struct onk_token_t tokens[16];
    uint16_t i=0;
    
    CuAssertTrue(tc, onk_tokenize("  1234  ", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, tokens[0].start == 2);
    CuAssertTrue(tc, tokens[0].end == 5);
    CuAssertTrue(tc, tokens[0].type == ONK_INTEGER_TOKEN);
    CuAssertTrue(tc, i == 2);
}

void __test__destroy_comment(CuTest* tc)
{
    struct onk_token_t tokens[16];
    uint16_t i=0;

    CuAssertTrue(tc, onk_tokenize("1234 # a very long comment", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, tokens[0].type == ONK_INTEGER_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 3);
    CuAssertTrue(tc, i == 2);
}

void __test__collapse_string(CuTest* tc)
{
    struct onk_token_t tokens[16];
    uint16_t i=0;

    CuAssertTrue(tc, onk_tokenize("\"1234\"", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 5);
    CuAssertTrue(tc, tokens[0].type == ONK_STRING_LITERAL_TOKEN);
    CuAssertTrue(tc, i == 2);

}

void __test__fails_on_partial_string(CuTest* tc)
{
    struct CompileTimeError error;
    struct onk_token_t tokens[16];
    uint16_t i=0;
    CuAssertTrue(tc, onk_tokenize("\"1234", tokens, &i, 16, false, &error) == -1);
}

void __test__num_var(CuTest* tc)
{
    struct onk_token_t tokens[16];
    uint16_t i = 0;
    CuAssertTrue(tc, onk_tokenize("5a", tokens, &i, 16, false, NULL) == 0);    
    CuAssertTrue(tc, i == 3);
    CuAssertTrue(tc, tokens[0].type == ONK_INTEGER_TOKEN);
    CuAssertTrue(tc, tokens[1].type == ONK_WORD_TOKEN);
    
}

void __test__collapse_word(CuTest* tc)
{
    struct onk_token_t tokens[16];
    uint16_t i = 0;
    CuAssertTrue(tc, onk_tokenize("abc", tokens, &i, 16, false, NULL) == 0);    
    
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == ONK_WORD_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 2);
    i=0;

    CuAssertTrue(tc, onk_tokenize("a", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == ONK_WORD_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 0);
    i=0;

    CuAssertTrue(tc, onk_tokenize(" a ", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == ONK_WORD_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 1);
    CuAssertTrue(tc, tokens[0].end == 1);
    i=0;

    CuAssertTrue(tc, onk_tokenize("a1_", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == ONK_WORD_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 2);
    i=0;

    CuAssertTrue(tc, onk_tokenize("a_1", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == ONK_WORD_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 2);

    CuAssertTrue(tc, onk_tokenize("_", tokens, &i, 16, false, NULL) == 0);
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == ONK_WORD_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 0);
}

void __test__collapse_operator(CuTest* tc)
{
    uint16_t sz=0;
    struct onk_token_t tokens[16];
    static enum onk_lexicon_t answers[] = {
        ONK_LT_EQL_TOKEN,
        ONK_GT_EQL_TOKEN,
        ONK_ISEQL_TOKEN,
        ONK_NOT_EQL_TOKEN,
        ONK_PLUSEQ_TOKEN,
        ONK_MINUS_EQL_TOKEN,
        ONK_AND_TOKEN,
        ONK_OR_TOKEN,
        ONK_SHR_TOKEN,
        ONK_SHL_TOKEN,
        ONK_BIT_OR_EQL,
        ONK_BIT_AND_EQL,
        ONK_BIT_NOT_EQL,
        0
    };

    static char * src_code[] =  {
        "<=",
        ">=",
        "==",
        "!=",
        "+=",
        "-=",
        "&&",
        "||",
        ">>",
        "<<",
        "|=",
        "&=",
        "~=",
        "|>",
        0
    };

    char msg[64];
    for (uint16_t i=0; 14 > i; i++) {
        sprintf(msg, "failed on idx %d \"%s\"", i, src_code[i]);
        CuAssert(tc, msg, onk_tokenize(src_code[i], tokens, &sz, 16, false, NULL) == 0);
        CuAssert(tc, msg, sz == 2);
        CuAssert(tc, msg, tokens[0].end == 1);
        CuAssert(tc, msg, tokens[0].start == 0);
        
        sprintf(msg, "expected <%s>, got <%s>", onk_ptoken(answers[i]), onk_ptoken(tokens[0].type));

        CuAssert(tc, msg, tokens[0].type == answers[i]);
        memset(msg, 0, 64);
        sz=0;
    }
}

void __test__position(CuTest* tc)
{
    uint16_t i=0;
    struct onk_token_t tokens[16];
    CuAssertTrue(tc, onk_tokenize("1234 + 1234", tokens, &i, 16, false, NULL) == 0);

    CuAssertTrue(tc, i == 4);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 3);
    CuAssertTrue(tc, tokens[0].type == ONK_INTEGER_TOKEN);

    CuAssertTrue(tc, tokens[1].start == 5);
    CuAssertTrue(tc, tokens[1].end == 6);
    CuAssertTrue(tc, tokens[1].type == ONK_ADD_TOKEN);

    CuAssertTrue(tc, tokens[2].start == 8);
    CuAssertTrue(tc, tokens[2].end == 11);
    CuAssertTrue(tc, tokens[2].type == ONK_INTEGER_TOKEN);
}

void __test__fails_on_utf(CuTest* tc)
{
    struct onk_token_t tokens[2];
    char buf[2] = {0xC3, 0xff};
    uint16_t i=0; 
    CuAssertTrue(tc, onk_tokenize(buf, tokens, &i, 2, false, NULL) == -1);
}

/* test if the british pound symbol is accepted by the lexer */
void __test__accepts_tea(CuTest* tc)
{
    struct onk_token_t tokens[2];
    char buf[2] = {0xC3, 0xff};
    //uint16_t i=0;
    //CuAssertTrue(tc, onk_tokenize(buf, tokens, &i, 2, false, NULL) == -1);
}


void __test__oversized_bin_ops(CuTest* tc)
{
    uint16_t sz=0;
    struct onk_token_t tokens[16];
    char msg[1024];

    char * token_name;
    int msg_cursor = 10, j=0;
    static int sizes[] = {
        2, 2, 2, 2,
        2, 2, 2, 2,
        2, 2, 2, 3,
        3, 2, 2, 2,
        2, 2, 2
    };
    static enum onk_lexicon_t answers[][8] = {
        {ONK_LT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0}, {ONK_GT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0}, {ONK_ISEQL_TOKEN, LT, 0}, {ONK_ISEQL_TOKEN, GT, 0}, 
        {ONK_GT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0}, {ONK_LT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0}, {ONK_ISEQL_TOKEN, ONK_EQUAL_TOKEN, 0}, {ONK_NOT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_NOT_TOKEN, 0}, {ONK_ADD_TOKEN, ONK_PLUSEQ_TOKEN, 0}, {ONK_SUB_TOKEN, ONK_MINUS_EQL_TOKEN, 0}, {ONK_EQUAL_TOKEN, ONK_ADD_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_EQUAL_TOKEN, ONK_SUB_TOKEN, ONK_SUB_TOKEN, 0}, {ONK_PLUSEQ_TOKEN, ONK_EQUAL_TOKEN, 0}, {ONK_MINUS_EQL_TOKEN, ONK_EQUAL_TOKEN, 0}, {ONK_ISEQL_TOKEN, ONK_ADD_TOKEN, 0}, 
        {ONK_ISEQL_TOKEN, ONK_SUB_TOKEN, 0}, {ONK_AND_TOKEN, ONK_AMPER_TOKEN, 0}, {ONK_OR_TOKEN, ONK_PIPE_TOKEN, 0}, {ONK_SHR_TOKEN, ONK_GT_TOKEN, 0}, {ONK_SHR_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_SHL_TOKEN, ONK_EQUAL_TOKEN, 0}, {ONK_SHL_TOKEN, LT, 0}, {ONK_BIT_OR_EQL, ONK_EQUAL_TOKEN, 0}, {ONK_ISEQL_TOKEN, PIPE, 0},
        {OR, ONK_GT_TOKEN, 0}, 0
    };

    static char * src_code[] =  {
        "<==", ">==", "==<", "==>",
        ">==", "<==", "===", "!==",
        "==!", "++=", "--=", "=++",
        "=--", "+==", "-==", "==+",
        "==-", "&&&", "|||", 
        ">>>", ">>=", "<<=",
        "<<<", "|==", "==|",
        "~~=", "~==",
        "|>>", "||>", 0
    };
   
    for (int i=0 ;; i++) {
        if (answers[i] == 0 || src_code[i] == 0)
            break;
        
        CuAssertTrue(tc, onk_tokenize(src_code[i], tokens, &sz, 16, false, NULL) == 0);
    
        sprintf(msg, "failed on %d (size: %d)", i, sz);
        onk_assert_tokens(tc, src_code[i], msg, tokens, answers[i]);
        CuAssert(tc, msg, sizes[i] == sz);
        sz=0;
        memset(msg, 0, 1024);
    }
}

void __test__derive_keywords(CuTest* tc)
{
    uint16_t sz=0;
    struct onk_token_t tokens[16];
    static enum onk_lexicon_t answers[] = {
        ONK_IF_TOKEN,
        ONK_ELSE_TOKEN,
        ONK_DEF_TOKEN,
        ONK_IMPL_TOKEN,
        EXTERN,
        ONK_RETURN_TOKEN,
        ONK_IMPORT_TOKEN,
        CONST,
        STATIC,
        AND,
        OR,
        0
    };

    static char * src_code[] =  {
        "if",
        "else",
        "def",
        "impl",
        "extern",
        "return",
        "import",
        "const",
        "static",
        "and",
        "or",
        0
    };
    
    char msg[64];
    for (uint16_t i=0; 11 > i; i++) {
        CuAssertTrue(tc, onk_tokenize(src_code[i], tokens, &sz, 16, false, NULL) == 0);
        
        CuAssertTrue(tc, sz == 1);
        sprintf(msg, "expected <%s>, got <%s>", onk_ptoken(answers[i]), onk_ptoken(tokens[0].type));

        CuAssert(tc, msg, tokens[0].type == answers[i]);
        memset(msg, 0, 64);
        sz=0;
    }
}

void __test__correct_tokenization(CuTest* tc)
{
    static char * src_code = "[]{}()!+- ><*/^%=&|:;_ 5a,~";    
    static enum onk_lexicon_t answers[] = {
        ONK_BRACKET_OPEN_TOKEN, ONK_BRACKET_CLOSE_TOKEN, 
        ONK_BRACE_OPEN_TOKEN, ONK_BRACE_CLOSE_TOKEN,
        ONK_PARAM_OPEN_TOKEN, ONK_PARAM_CLOSE_TOKEN,
        ONK_NOT_TOKEN, ONK_ADD_TOKEN, ONK_SUB_TOKEN,
        ONK_GT_TOKEN, ONK_LT_TOKEN, ONK_MUL_TOKEN,
        ONK_DIV_TOKEN, ONK_POW_TOKEN, ONK_MOD_TOKEN,
        ONK_EQUAL_TOKEN, AMPER, PIPE,
        ONK_COLON_TOKEN, ONK_SEMICOLON_TOKEN,
        ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_WORD_TOKEN,
        ONK_COMMA_TOKEN, ONK_TILDE_TOKEN, 0
    };

    uint16_t sz=0, temp=0;
    struct onk_token_t tokens[32];
    char msg[64];

    CuAssertTrue(tc, onk_tokenize(src_code, tokens, &sz, 32, false, NULL) == 0);
    CuAssertTrue(tc, sz == 25);
    
    for (uint16_t i=0; sz > i; i++) {
        sprintf(msg, "expected <%s>, got <%s> [%d]", onk_ptoken(answers[i]), onk_ptoken(tokens[i].type), i);

        CuAssert(tc, msg, tokens[i].type == answers[i]);
        memset(msg, 0, 64);
    }
}

void __test__negative_num_var(CuTest* tc) {
    uint16_t ntokens=0;
    struct onk_token_t tokens[8];
    enum onk_lexicon_t answer_1[8] = {ONK_INTEGER_TOKEN, 0};
    enum onk_lexicon_t answer_2[8] = {ONK_INTEGER_TOKEN, ONK_SUB_TOKEN, ONK_INTEGER_TOKEN, 0};

    CuAssertTrue(tc, onk_tokenize("-1234", tokens, &ntokens, 8, false, NULL) == 0);
    onk_assert_tokens(tc, "-1234", "", tokens, answer_1);
    CuAssertTrue(tc, ntokens == 2);
    
    ntokens = 0;
    CuAssertTrue(tc, onk_tokenize("1234 - 1234", tokens, &ntokens, 8, false, NULL) == 0);
    onk_assert_tokens(tc, "1234 - 1234", "", tokens, answer_2);
    CuAssertTrue(tc, ntokens == 4);
    
    ntokens = 0;
    CuAssertTrue(tc, onk_tokenize("1234- -1234", tokens, &ntokens, 8, false, NULL) == 0);
    onk_assert_tokens(tc, "1234- -1234", "", tokens, answer_2);
    CuAssertTrue(tc, ntokens == 4);    
    
    ntokens = 0;
    CuAssertTrue(tc, onk_tokenize("-1234--1234", tokens, &ntokens, 8, false, NULL) == 0);
    onk_assert_tokens(tc, "-1234--1234", "", tokens, answer_2);
    CuAssertTrue(tc, ntokens == 4);    
}

void __test__underscored_number(CuTest* tc) {
    uint16_t ntokens=0;
    const char *src = "1_234";
    struct onk_token_t tokens[8];
    enum onk_lexicon_t answer[4] = {ONK_INTEGER_TOKEN, 0};

    CuAssertTrue(tc, onk_tokenize(src, tokens, &ntokens, 8, false, NULL) == 0);
    CuAssertTrue(tc, ntokens == 2);    
    onk_assert_tokens(tc, src, "", tokens, answer);
}


void __test__string_nested_quoted(CuTest* tc) {
    uint16_t ntokens=0;
    const char *src = "\"\\\"\""; /* "\"" */
    struct onk_token_t tokens[8];
    enum onk_lexicon_t answer[4] = {ONK_STRING_LITERAL_TOKEN, 0};

    CuAssertTrue(tc, onk_tokenize(src, tokens, &ntokens, 8, false, NULL) == 0);
    CuAssertTrue(tc, ntokens == 2);
    onk_assert_tokens(tc, src, "", tokens, answer);
}

CuSuite* LexerUnitTestSuite(void) {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__fails_on_utf);
    SUITE_ADD_TEST(suite, __test__collapse_integer);
    SUITE_ADD_TEST(suite, __test__num_var);
    SUITE_ADD_TEST(suite, __test__negative_num_var);
    SUITE_ADD_TEST(suite, __test__fails_on_partial_string);
    SUITE_ADD_TEST(suite, __test__oversized_bin_ops);
    SUITE_ADD_TEST(suite, __test__destroy_whitespace);
    SUITE_ADD_TEST(suite, __test__destroy_comment);
    SUITE_ADD_TEST(suite, __test__collapse_string);
    SUITE_ADD_TEST(suite, __test__collapse_word);
    SUITE_ADD_TEST(suite, __test__collapse_operator);
    SUITE_ADD_TEST(suite, __test__underscored_number);

    SUITE_ADD_TEST(suite, __test__basic_perthensis);
    SUITE_ADD_TEST(suite, __test__derive_keywords);
    SUITE_ADD_TEST(suite, __test__string_nested_quoted);
    SUITE_ADD_TEST(suite, __test__correct_tokenization);
    return suite;
}
