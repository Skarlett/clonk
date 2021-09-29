#include <stdio.h>
#include <string.h>

#include "../CuTest.h"
#include "../../src/prelude.h"
#include "../../src/parser/lexer/lexer.h"
#include "../../src/parser/lexer/helpers.h"
#include "../../src/parser/error.h"
#include "../common.h"


void __test__basic_perthensis(CuTest* tc)
{
    static usize tokens_sz[] = {5, 7, 7, 9, 9, 11, 13};
    usize ntokens=0;

    struct Token tokens[16];
    char msg[1028];


    static char * line[] =  {
        "(1 + 2)",        // 5
        "(1 + 2) + 3",    // 7
        "1 + (2 + 3)",    // 7
        "(1 + (2 + 3))",        // 9
        "((1 + 2) + 3)",         // 9,
        "(1 + 2) + (3 + 4)",     // 10,
        "((1 + 2) + (3 + 4))",   // 12,
        0
    };
    
    static enum Lexicon check_list[][16] = {
        {PARAM_OPEN, INTEGER, ADD, INTEGER, PARAM_CLOSE},
        {PARAM_OPEN, INTEGER, ADD, INTEGER, PARAM_CLOSE, ADD, INTEGER},
        {INTEGER, ADD, PARAM_OPEN, INTEGER, ADD, INTEGER, PARAM_CLOSE},
        {PARAM_OPEN, INTEGER, ADD, PARAM_OPEN, INTEGER, ADD, INTEGER, PARAM_CLOSE, PARAM_CLOSE},
        {PARAM_OPEN, PARAM_OPEN, INTEGER, ADD, INTEGER, PARAM_CLOSE, ADD, INTEGER, PARAM_CLOSE},
        {PARAM_OPEN, INTEGER, ADD, INTEGER, PARAM_CLOSE, ADD, PARAM_OPEN, INTEGER, ADD, INTEGER, PARAM_CLOSE},
        {PARAM_OPEN, PARAM_OPEN, INTEGER, ADD, INTEGER, PARAM_CLOSE, ADD, PARAM_OPEN, INTEGER, ADD, INTEGER, PARAM_CLOSE, PARAM_CLOSE}
    };

    for (usize i=0; 7 > i; i++) {
        CuAssertTrue(tc, tokenize(line[i], tokens, &ntokens, NULL) == 0);
        CuAssertTrue(tc, ntokens == tokens_sz[i]);

        char got[512];
        char expected[512];
        char token_buf[24];

        int offset_got = 0;
        int offset_expected = 0;
        int tmp = 0;

        // Setup error messages for tests
        for (usize j = 0; tokens_sz[i] > j; j++) {
            sprintf(token_buf, "[%s] ", ptoken(tokens[j].type));
            tmp = strlen(ptoken(tokens[j].type)) + 3;
            strncpy(got + offset_got, token_buf, tmp);
            offset_got += tmp;

            memset(&token_buf, 0, 24);

            sprintf(token_buf, "[%s] ", ptoken(check_list[i][j]));
            tmp = strlen(ptoken(tokens[j].type)) + 3;
            strncpy(expected + offset_expected, token_buf, tmp);
            offset_expected += tmp;
            memset(&token_buf, 0, 24);

        }

        sprintf(msg,"Expected [%s], got [%s] | * %s *", expected, got, line[i]);        
        CuAssert(tc, msg, __check_tokens(tokens, check_list[i], tokens_sz[i]));
        ntokens=0;
    }
}

void __test__collapse_integer(CuTest* tc)
{ 
    struct Token tokens[16];
    usize i=0;

    CuAssertTrue(tc, tokenize("1234", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, i == 1);
    CuAssertTrue(tc, tokens[0].type == INTEGER);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 3);
    i=0;

    CuAssertTrue(tc, tokenize("1", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, i == 1);
    CuAssertTrue(tc, tokens[0].type == INTEGER);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 0);
    i=0;

    CuAssertTrue(tc, tokenize(" 1 ", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, i == 1);
    CuAssertTrue(tc, tokens[0].type == INTEGER);
    CuAssertTrue(tc, tokens[0].start == 1);
    CuAssertTrue(tc, tokens[0].end == 1);
}

void __test__destroy_whitespace(CuTest* tc)
{
    struct Token tokens[16];
    usize i=0;
    
    CuAssertTrue(tc, tokenize("  1234  ", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, tokens[0].start == 2);
    CuAssertTrue(tc, tokens[0].end == 5);
    CuAssertTrue(tc, tokens[0].type == INTEGER);
    CuAssertTrue(tc, i == 1);
}

void __test__destroy_comment(CuTest* tc)
{
    struct Token tokens[16];
    usize i=0;

    CuAssertTrue(tc, tokenize("1234 # a very long comment", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, tokens[0].type == INTEGER);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 3);
    CuAssertTrue(tc, i == 1);
}

void __test__collapse_string(CuTest* tc)
{
    struct Token tokens[16];
    usize i=0;

    CuAssertTrue(tc, tokenize("\"1234\"", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 5);
    CuAssertTrue(tc, tokens[0].type == STRING_LITERAL);
    CuAssertTrue(tc, i == 1);

}

void __test__fails_on_partial_string(CuTest* tc)
{
    struct CompileTimeError error;
    struct Token tokens[16];
    usize i=0;
    CuAssertTrue(tc, tokenize("\"1234", tokens, &i, &error) == -1);
}

void __test__num_var(CuTest* tc)
{
    struct Token tokens[16];
    usize i = 0;
    CuAssertTrue(tc, tokenize("5a", tokens, &i, NULL) == 0);    
    CuAssertTrue(tc, i == 2);
    CuAssertTrue(tc, tokens[0].type == INTEGER);
    CuAssertTrue(tc, tokens[1].type == WORD);
    
}

void __test__collapse_word(CuTest* tc)
{
    struct Token tokens[16];
    usize i = 0;
    CuAssertTrue(tc, tokenize("abc", tokens, &i, NULL) == 0);    
    
    CuAssertTrue(tc, i == 1);
    CuAssertTrue(tc, tokens[0].type == WORD);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 2);
    i=0;

    CuAssertTrue(tc, tokenize("a", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, i == 1);
    CuAssertTrue(tc, tokens[0].type == WORD);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 0);
    i=0;

    CuAssertTrue(tc, tokenize(" a ", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, i == 1);
    CuAssertTrue(tc, tokens[0].type == WORD);
    CuAssertTrue(tc, tokens[0].start == 1);
    CuAssertTrue(tc, tokens[0].end == 1);
    i=0;

    CuAssertTrue(tc, tokenize("a1_", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, i == 1);
    CuAssertTrue(tc, tokens[0].type == WORD);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 2);
    i=0;

    CuAssertTrue(tc, tokenize("a1_", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, i == 1);
    CuAssertTrue(tc, tokens[0].type == WORD);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 2);
    i=0;

    CuAssertTrue(tc, tokenize("a_1", tokens, &i, NULL) == 0);
    CuAssertTrue(tc, i == 1);
    CuAssertTrue(tc, tokens[0].type == WORD);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 2);
}

void __test__collapse_operator(CuTest* tc)
{
    
    usize sz=0;
    struct Token tokens[16];
    static enum Lexicon answers[] = {
        LTEQ,
        GTEQ,
        ISEQL,
        ISNEQL,

        PLUSEQ,
        MINUSEQ,
        AND,
        OR
    };

    static char * line[] =  {
        "<=",
        ">=",
        "==",
        "!=",
        "+=",
        "-=",
        "&&",
        "||",
        0
    };

    char msg[64];
    for (usize i=0; 8 > i; i++) {
        CuAssertTrue(tc, tokenize(line[i], tokens, &sz, NULL) == 0);
        CuAssertTrue(tc, sz == 1);
        CuAssertTrue(tc, tokens[0].end == 1);
        CuAssertTrue(tc, tokens[0].start == 0);
        
        sprintf(msg, "expected <%s>, got <%s>", ptoken(answers[i]), ptoken(tokens[0].type));

        CuAssert(tc, msg, tokens[0].type == answers[i]);
        memset(msg, 0, 64);
        sz=0;
    }
}

void __test__position(CuTest* tc)
{
    usize i=0;
    struct Token tokens[16];
    CuAssertTrue(tc, tokenize("1234 + 1234", tokens, &i, NULL) == 0);

    CuAssertTrue(tc, i == 3);
    CuAssertTrue(tc, tokens[0].start == 0);
    CuAssertTrue(tc, tokens[0].end == 3);
    CuAssertTrue(tc, tokens[0].type == INTEGER);

    CuAssertTrue(tc, tokens[1].start == 5);
    CuAssertTrue(tc, tokens[1].end == 6);
    CuAssertTrue(tc, tokens[1].type == ADD);

    CuAssertTrue(tc, tokens[2].start == 8);
    CuAssertTrue(tc, tokens[2].end == 11);
    CuAssertTrue(tc, tokens[2].type == INTEGER);
}

void __test__fails_on_utf(CuTest* tc)
{
    struct Token tokens[2];
    char buf[2] = {0xC3, 0xff};
    usize i=0;    
    CuAssertTrue(tc, tokenize(buf, tokens, &i, NULL) == -1);
}

void __test__oversized_bin_ops(CuTest* tc)
{
    usize sz=0;
    struct Token tokens[16];
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
    static enum Lexicon answers[][4] = {
        {LTEQ, EQUAL}, {GTEQ, EQUAL}, {ISEQL, LT}, {ISEQL, GT}, 
        {GTEQ, EQUAL}, {LTEQ, EQUAL}, {ISEQL, EQUAL}, {ISNEQL, EQUAL},
        {ISEQL, NOT}, {ADD, PLUSEQ}, {SUB, MINUSEQ}, {EQUAL, ADD, ADD},
        {EQUAL, SUB, SUB}, {PLUSEQ, EQUAL}, {MINUSEQ, EQUAL}, {ISEQL, ADD}, 
        {ISEQL, SUB}, {AND, AMPER}, {OR, PIPE}
    };

    static char * line[] =  {
        "<==", ">==", "==<", "==>",
        ">==", "<==", "===", "!==",
        "==!", "++=", "--=", "=++",
        "=--", "+==", "-==", "==+",
        "==-", "&&&", "|||", 0
    };
   
    for (int i=0; 19 > i; i++) {
        CuAssertTrue(tc, tokenize(line[i], tokens, &sz, NULL) == 0);
    
        sprintf(msg, "failed on %d (size: %ld)", i, sz);

        CuAssert(tc, msg, sizes[i] == sz);
        CuAssert(tc, msg, __check_tokens(tokens, answers[i], sz));
        sz=0;
        memset(msg, 0, 1024);
    }
}

void __test__derive_keywords(CuTest* tc)
{
    usize sz=0;
    struct Token tokens[16];
    static enum Lexicon answers[] = {
        IF,
        ELSE,
        FUNC_DEF,
        IMPL,
        EXTERN,
        RETURN,
        IMPORT,
        CONST,
        STATIC,
        AND,
        OR
    };

    static char * line[] =  {
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
    for (usize i=0; 11 > i; i++) {
        CuAssertTrue(tc, tokenize(line[i], tokens, &sz, NULL) == 0);
        
        CuAssertTrue(tc, sz == 1);
        sprintf(msg, "expected <%s>, got <%s>", ptoken(answers[i]), ptoken(tokens[0].type));

        CuAssert(tc, msg, tokens[0].type == answers[i]);
        memset(msg, 0, 64);
        sz=0;
    }
}

void __test__correct_tokenization(CuTest* tc)
{
    static char * line = "[]{}()!+- ><*/^%=&|:;_ 5a,";    
    static enum Lexicon answers[] = {
        BRACKET_OPEN, BRACKET_CLOSE, 
        BRACE_OPEN, BRACE_CLOSE,
        PARAM_OPEN, PARAM_CLOSE,
        NOT, ADD, SUB,
        GT, LT, MUL,
        DIV, POW, MOD,
        EQUAL, AMPER, PIPE,
        COLON, SEMICOLON,
        WORD, INTEGER, WORD,
        COMMA
    };

    usize sz=0, temp=0;
    struct Token tokens[32];
    char msg[64];
    
    CuAssertTrue(tc, tokenize(line, tokens, &sz, NULL) == 0);
    CuAssertTrue(tc, sz == 24);
    
    for (usize i=0; sz > i; i++) {
        sprintf(msg, "expected <%s>, got <%s> [%ld]", ptoken(answers[i]), ptoken(tokens[i].type), i);

        CuAssert(tc, msg, tokens[i].type == answers[i]);
        memset(msg, 0, 64);
    }
}
void __test__num_is_negative(CuTest* tc) {
    usize ntokens=0;
    struct Token tokens[32];

    CuAssertTrue(tc, tokenize("-1234", tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 1);

    CuAssertTrue(tc, tokenize("1234 - 1234", tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 3);

    CuAssertTrue(tc, tokenize("1234- -1234", tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 3);

    CuAssertTrue(tc, tokenize("-1234--1234", tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 3);
    
}

CuSuite* LexerUnitTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, __test__fails_on_utf);
    SUITE_ADD_TEST(suite, __test__collapse_integer);
    SUITE_ADD_TEST(suite, __test__num_var);
    SUITE_ADD_TEST(suite, __test__fails_on_partial_string);
    SUITE_ADD_TEST(suite, __test__oversized_bin_ops);
	SUITE_ADD_TEST(suite, __test__destroy_whitespace);
    SUITE_ADD_TEST(suite, __test__destroy_comment);
	SUITE_ADD_TEST(suite, __test__collapse_string);
    SUITE_ADD_TEST(suite, __test__collapse_word);
    SUITE_ADD_TEST(suite, __test__collapse_operator);
    SUITE_ADD_TEST(suite, __test__num_is_negative);
    

    SUITE_ADD_TEST(suite, __test__basic_perthensis);
    SUITE_ADD_TEST(suite, __test__derive_keywords);

    SUITE_ADD_TEST(suite, __test__correct_tokenization);
    return suite;
}