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

int8_t _filter_cfg(test_flag flags, enum onk_lexicon_t current, uint16_t tokens_len, uint8_t i)
{
    if ((flags & ignore_whitespace && onk_is_tok_whitespace(current))
      ||(flags & ignore_eof && current == ONK_EOF_TOKEN)
      ||(flags & ignore_comments && current == ONK_COMMENT_TOKEN))
      return 1;
    return 0;
}

void lexer_harness(
    CuTest *tc,
    struct onk_lexer_output_t *err_output,
    const char* const* src_code,
    enum onk_lexicon_t **answers,
    uint8_t flags,
    const char * fp,
    uint16_t line
){
    struct onk_lexer_input_t lexer_input;
    struct onk_lexer_output_t output;

    enum onk_lexicon_t current = 0;
    char msg[512];
    uint16_t actr = 0;
    int8_t lex_ret = 0;

    for (uint8_t set=0; MAX_SETS > set; set++)
    {
        if(src_code[set] == 0 || answers[set] == 0)
          return;

        for(uint16_t actr=0; MAX_ANSW_LEN > actr; actr++)
            if (answers[set][actr] == 0)
                break;

        lexer_input.src_code = src_code[set];
        lex_ret = onk_tokenize(&lexer_input, err_output);

        CuAssertTrue(tc, err_output->tokens.len == actr);
        CuAssertTrue(tc, lex_ret != 0);

        /* check every token */
        for (uint16_t i=0; err_output->tokens.len > i; i++)
        {
            current = ((struct onk_token_t *)
                       err_output->tokens.base)[i].type;

            if(_filter_cfg(flags, current, err_output->tokens.len, i))
                continue;

            if(current != answers[set][i])
            {
                sprintf(msg, "%s:%u failed on set idx[%d]", fp, line, i);
                CuFail(tc, msg);
            }
        }

        onk_vec_clear(&output.tokens);
    }

    return;
}

#define LexHarness(tc, out, src, answer, flags) \
    lexer_harness((tc), (out), (src), (answer), (flags), __FILE__, __LINE__)

void __test__io(CuTest* tc)
{
    struct onk_lexer_output_t lex_output;
    const char * src[] = {
        "a",
        "aa",
        "1a"
        "a1",
        "a_1"
        "_"
        "10",
        "1_0",
        "1",
        " 1 ",
        "1234",
        "-1234",
        "\"1234\"",
        "\"12\\\"34\"", // "12\"43"
        "£",
        "$",
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
        "<==", ">==", "==<", "==>",
        ">==", "<==", "===", "!==",
        "==!", "++=", "--=", "=++",
        "=--", "+==", "-==", "==+",
        "==-", "&&&", "|||",
        ">>>", ">>=", "<<=",
        "<<<", "|==", "==|",
        "~~=", "~==",

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
        "[]{}()!+- ><*/^%=&|:;_ 5a,~\\",

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

    enum onk_lexicon_t answers[][32] = {
        {ONK_WORD_TOKEN, 0},
        {ONK_WORD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_WORD_TOKEN, 0},
        {ONK_WORD_TOKEN, 0},
        {ONK_WORD_TOKEN, 0},
        {ONK_WORD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, 0},

        {ONK_STRING_LITERAL_TOKEN, 0},
        {ONK_STRING_LITERAL_TOKEN, 0},

        {ONK_DOLLAR_TOKEN, 0},
        {ONK_DOLLAR_TOKEN, 0},

        {ONK_LT_EQL_TOKEN, 0},
        {ONK_GT_EQL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, 0},
        {ONK_NOT_EQL_TOKEN, 0},
        {ONK_PLUSEQ_TOKEN, 0},
        {ONK_MINUS_EQL_TOKEN, 0},
        {ONK_AND_TOKEN, 0},
        {ONK_OR_TOKEN, 0},
        {ONK_SHR_TOKEN, 0},
        {ONK_SHL_TOKEN, 0},
        {ONK_BIT_OR_EQL, 0},
        {ONK_BIT_AND_EQL, 0},
        {ONK_BIT_NOT_EQL, 0},

        {ONK_LT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_GT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_LT_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_GT_TOKEN, 0},
        {ONK_GT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_LT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_NOT_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_NOT_TOKEN, 0},
        {ONK_ADD_TOKEN, ONK_PLUSEQ_TOKEN, 0},
        {ONK_SUB_TOKEN, ONK_MINUS_EQL_TOKEN, 0},
        {ONK_EQUAL_TOKEN, ONK_ADD_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_EQUAL_TOKEN, ONK_SUB_TOKEN, ONK_SUB_TOKEN, 0},
        {ONK_PLUSEQ_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_MINUS_EQL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_ISEQL_TOKEN, ONK_SUB_TOKEN, 0},
        {ONK_AND_TOKEN, ONK_AMPER_TOKEN, 0},
        {ONK_OR_TOKEN, ONK_PIPE_TOKEN, 0},
        {ONK_SHR_TOKEN, ONK_GT_TOKEN, 0},
        {ONK_SHR_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_SHL_TOKEN, ONK_EQUAL_TOKEN, 0},
        {ONK_SHL_TOKEN, ONK_LT_TOKEN, 0},
        {ONK_BIT_OR_EQL, ONK_EQUAL_TOKEN, 0},
        {ONK_OR_TOKEN, ONK_GT_TOKEN, 0},

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
        {0},
    };

    LexHarness(
        tc, &lex_output, src, (enum onk_lexicon_t **)answers,
        ignore_whitespace | ignore_comments | ignore_eof
    );
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
    int8_t ret = 0;
    uint16_t i=0;

    input.src_code = "1234 + 4321";

    ret = onk_tokenize(&input, &output);
    tokens = output.tokens.base;

    CuAssertTrue(tc, ret == -1);

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
    struct onk_lexer_input_t input;
    struct onk_lexer_output_t output;
    const unsigned char buf[] = {0xC3, 0xff, 0x00};
    int8_t ret = 0;

    input.src_code = (const char *)buf;
    ret = onk_tokenize(&input, &output);
    CuAssertTrue(tc, ret == -1);
}

CuSuite* LexerUnitTests(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__io);
    SUITE_ADD_TEST(suite, __test__destroy_comment);
    SUITE_ADD_TEST(suite, __test__position);
    SUITE_ADD_TEST(suite, __test__fails_on_partial_string);
    SUITE_ADD_TEST(suite, __test__fails_on_utf);
    return suite;
}
