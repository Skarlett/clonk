#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "libtest/CuTest.h"
#include "libtest/assert.h"
#include "lexer.h"

#define MAX_SETS 64
#define MAX_ANSW_LEN 64

#define ignore_whitespace       1
#define ignore_eof              4
#define ignore_comments         8

typedef uint8_t test_flag;

#define MESSAGE_BUF_SZ 16
void __test__io(CuTest* tc)
{
    struct onk_lexer_output_t lex_output;
    const char * src[] = {
        "a",
        "aa",
        "1a",
        "a1",
        "a_1", // 5

        "_",
        "10",
        "1_0",
        "1",
        " 1 ", // 10
        
        "1234",
        "-1234",
        "\"1234\"",
        "<=",
        ">=",
        
        "==",
        "!=",

        "+=",
        "-=",
        
        "&&", // 20
        "||",

        ">>",
        "<<", // 23
        "|=", // 24
        "&=",//25
        "~=",
        "<==", ">==", "==<", "==>",
        ">==", "<==", "===", "!==",
        "==!", "++=", "--=", "=++",
        "=--", "+==", "-==", "==+",
        "==-", "&&&", "|||",
        ">>>", ">>=", "<<=",
        "<<<", "|==", "==|",
        //"~~=", "~==",

        "if",
        "else",
        "def",
        "impl",
        // "extern",
        "return",
        "import",
        "from",
        "and",
        "or",
        "continue",
        "break",
        "while",
        "for",
        "in",
        "[]{}()!+- ><*/^%=&|:;_ 5a,~\\${",

        "1 + 2",
        "(1 + 2)",        // 5
        "(1 + 2) + 3",    // 7
        "1 + (2 + 3)",    // 7
        "(1 + (2 + 3))",        // 9
        "((1 + 2) + 3)",         // 9,
        "(1 + 2) + (3 + 4)",     // 10,
        "((1 + 2) + (3 + 4))",   // 12,
        0
    };
    enum onk_lexicon_t *refanswer = 0;
    
    enum onk_lexicon_t answers[78][32] = {
        {ONK_WORD_TOKEN, 0},
        {ONK_WORD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_WORD_TOKEN, 0},        
        {ONK_WORD_TOKEN, 0},
        {ONK_WORD_TOKEN, 0}, // 5

        {ONK_WORD_TOKEN, 0},        
        {ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, 0}, // 10

        {ONK_INTEGER_TOKEN, 0},         
        {ONK_INTEGER_TOKEN, 0},
        {ONK_STRING_LITERAL_TOKEN, 0},
        
        {ONK_LT_EQL_TOKEN, 0},
        {ONK_GT_EQL_TOKEN, 0}, 
        {ONK_ISEQL_TOKEN, 0},
        {ONK_NOT_EQL_TOKEN, 0},
        {ONK_PLUSEQ_TOKEN, 0},
        {ONK_MINUS_EQL_TOKEN, 0},

        {ONK_AND_TOKEN, 0}, // 20
        {ONK_OR_TOKEN, 0},
        {ONK_SHR_TOKEN, 0},
        {ONK_SHL_TOKEN, 0},
        {ONK_BIT_OR_EQL, 0},
        {ONK_BIT_AND_EQL, 0}, // 25
        {ONK_BIT_NOT_EQL, 0},

        {ONK_LT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_GT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_LT_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_GT_TOKEN, 0},
        {ONK_GT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0}, // 30

        {ONK_LT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_NOT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_NOT_TOKEN, 0},
        {ONK_ADD_TOKEN, ONK_PLUSEQ_TOKEN, 0}, // 35

        {ONK_SUB_TOKEN, ONK_MINUS_EQL_TOKEN, 0},
        {ONK_EQUAL_TOKEN, ONK_ADD_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_EQUAL_TOKEN, ONK_SUB_TOKEN, ONK_SUB_TOKEN, 0},
        {ONK_PLUSEQ_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_MINUS_EQL_TOKEN, ONK_EQUAL_TOKEN, 0}, //40
        
        {ONK_ISEQL_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_SUB_TOKEN, 0},
        {ONK_AND_TOKEN, ONK_AMPER_TOKEN, 0},
        {ONK_OR_TOKEN, ONK_PIPE_TOKEN, 0},
        {ONK_SHR_TOKEN, ONK_GT_TOKEN, 0}, // 45

        {ONK_SHR_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_SHL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_SHL_TOKEN, ONK_LT_TOKEN, 0},
        {ONK_BIT_OR_EQL, ONK_EQUAL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_PIPE_TOKEN, 0},

        {ONK_IF_TOKEN, 0},
        {ONK_ELSE_TOKEN, 0},
        {ONK_DEF_TOKEN, 0},
        {ONK_IMPL_TOKEN, 0},
        // {EXTERN, 0},
        {ONK_RETURN_TOKEN, 0},
        {ONK_IMPORT_TOKEN, 0},
        {ONK_FROM_TOKEN, 0},
        {ONK_AND_TOKEN, 0},
        {ONK_OR_TOKEN, 0},
        {ONK_CONTINUE_TOKEN, 0},
        {ONK_BREAK_TOKEN, 0},
        {ONK_WHILE_TOKEN, 0},
        {ONK_FOR_TOKEN, 0},
        {ONK_IN_TOKEN, 0},
        {
          ONK_BRACKET_OPEN_TOKEN, ONK_BRACKET_CLOSE_TOKEN,
          ONK_BRACE_OPEN_TOKEN, ONK_BRACE_CLOSE_TOKEN,
          ONK_PARAM_OPEN_TOKEN, ONK_PARAM_CLOSE_TOKEN,
          ONK_NOT_TOKEN, ONK_ADD_TOKEN, ONK_SUB_TOKEN,
          ONK_GT_TOKEN, ONK_LT_TOKEN, ONK_MUL_TOKEN,
          ONK_DIV_TOKEN, ONK_POW_TOKEN, ONK_MOD_TOKEN,
          ONK_EQUAL_TOKEN, ONK_AMPER_TOKEN, ONK_PIPE_TOKEN,
          ONK_COLON_TOKEN, ONK_SEMICOLON_TOKEN,
          ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_WORD_TOKEN,
          ONK_COMMA_TOKEN, ONK_TILDE_TOKEN, ONK_BACKSLASH_TOKEN,
          ONK_HASHMAP_LITERAL_START_TOKEN,
          0
        },

        {ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_ADD_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        {ONK_PARAM_OPEN_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_ADD_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_PARAM_CLOSE_TOKEN, ONK_PARAM_CLOSE_TOKEN, 0},
        0,
    };

    struct onk_lexer_input_t lexer_input;
    struct onk_lexer_output_t output;
    
    enum onk_lexicon_t current = 0;
    char msg[MESSAGE_BUF_SZ];
    int msg_offset=0;
    int8_t lex_ret = 0;

    for (uint8_t set=0; MAX_SETS > set; set++)
    {
        if(answers[set] == 0)
          return;

        lexer_input.src_code = src[set];
        lex_ret = onk_tokenize(&lexer_input, &output);
        memset(msg, 0, MESSAGE_BUF_SZ);

        msg_offset=snprintf(msg, MESSAGE_BUF_SZ, "index <%u>", set);
        OnkAssertTokensMsg(tc, msg, output.tokens.base, answers[set]);
        
        CuAssert(tc, msg, lex_ret == 0);
        onk_vec_clear(&output.tokens);
    }    
}

#define mesg_sz 128
void __test__lexarr_strncat(CuTest *tc)
{
    char mesg[mesg_sz];
    uint16_t offset=0;
    enum onk_lexicon_t answers[] =
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, 0};
    static const char *expected = "[integer] [integer]";
    
    mesg[0] = 0;
    offset = onk_lexarr_strncat(
        mesg,
        mesg_sz,
        answers, 2
    );

    CuAssertIntEquals(tc, 0, strncmp(mesg, expected, strlen(expected)-1));
}

void __test__destroy_comment(CuTest* tc)
{
    struct onk_lexer_input_t input;
    struct onk_lexer_output_t output;
    struct onk_token_t *tokens = 0;
    int8_t ret = 0;

    input.src_code = "1234 # a very long comment";

    ret = onk_tokenize(&input, &output);
    CuAssertTrue(tc, ret == 0);

    tokens = output.tokens.base;

    CuAssertTrue(tc, tokens[0].type == ONK_INTEGER_TOKEN);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 3);
}

void __test__fails_on_partial_string(CuTest* tc)
{
    struct onk_lexer_input_t input;
    struct onk_lexer_output_t output;
    int8_t ret = 0;

    input.src_code = "\"1234";

    ret = onk_tokenize(&input, &output);
    CuAssertTrue(tc, ret == -1);
}

void __test__position(CuTest* tc)
{
    struct onk_lexer_input_t input;
    struct onk_lexer_output_t output;
    struct onk_token_t *tokens = 0;

    char mesg[512];
    int8_t ret = 0;
    uint16_t i=0;

    input.src_code = "1234 + 4321";

    ret = onk_tokenize(&input, &output);
    tokens = output.tokens.base;

    CuAssertTrue(tc, ret == 0);

    CuAssertIntEquals(tc, tokens[0].start, 0);
    CuAssertIntEquals(tc, tokens[0].end, 3);
    CuAssertTrue(tc, tokens[0].type == ONK_INTEGER_TOKEN);

    CuAssertIntEquals(tc, tokens[1].start, 5);
    CuAssertIntEquals(tc, tokens[1].end, 5);
    CuAssertTrue(tc, tokens[1].type == ONK_ADD_TOKEN);

    CuAssertIntEquals(tc, tokens[2].start, 7);
    CuAssertIntEquals(tc, tokens[2].end, 10);
    CuAssertTrue(tc, tokens[2].type == ONK_INTEGER_TOKEN);
}

void __test__fails_on_utf(CuTest* tc)
{
    struct onk_lexer_input_t input;
    struct onk_lexer_output_t output;
    const unsigned char buf[] = {0xC3, 0xff, 0x00};
    int8_t ret = 0;

    input.src_code = (const char *)buf;
    ret = onk_tokenize(&input, &output);
    CuAssertTrue(tc, ret == -1);
}

void __test__escape_quotes(CuTest *tc)
{
    CuFail(tc, "unimplemented");
}

void __test__undefined_lexicon_token(CuTest *tc) {
   CuFail(tc, "unimplemented");
}



CuSuite* LexerUnitTests(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__lexarr_strncat);
    SUITE_ADD_TEST(suite, __test__io);
    SUITE_ADD_TEST(suite, __test__destroy_comment);
    SUITE_ADD_TEST(suite, __test__position);
    SUITE_ADD_TEST(suite, __test__fails_on_partial_string);
    SUITE_ADD_TEST(suite, __test__fails_on_utf);
    SUITE_ADD_TEST(suite, __test__fails_on_partial_string);
    /* SUITE_ADD_TEST(suite, __test__escape_quotes); */
    /* SUITE_ADD_TEST(suite, __test__undefined_lexicon_token); */
    return suite;
}
