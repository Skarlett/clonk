#include <string.h>
#include <stdio.h>
#include "../../src/utils/vec.h"
#include "../../src/parser/lexer/lexer.h"
#include "../../src/parser/expr/expr.h"
#include "../../src/prelude.h"
#include "../../src/parser/error.h"
#include "../testutils.h"
#include "../CuTest.h"

int8_t into_ref_array(
    struct Token input[],
    struct Token *out[],
    const usize input_sz,
    const usize out_sz
){
    if (input_sz > out_sz)
        return -1;
    
    for (usize i=0; input_sz > i; i++)
        out[i] = &input[i];

    return 0;
}
#define __SIM_ORD_PRECEDENSE_MSG_BUF_SZ 128


void __test__tuple_collection(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
   
    static char * line[] = {
        "()",
        "(1)",
        "(1 + 2)",
        "(1, 2)",
        "(1, (2, 3))",
        "((1, 2), 3)",
        "((1, 2, 3))",
        "((((((1, 2, 3))))))",
    };

    static enum Lexicon check_list[][8] = {
        {TupleGroup, 0},
        {INTEGER, 0},
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, TupleGroup, 0},
        {INTEGER, INTEGER, INTEGER, TupleGroup, TupleGroup, 0},
        {INTEGER, INTEGER,  TupleGroup, INTEGER, TupleGroup, 0},
        {INTEGER, INTEGER, INTEGER, TupleGroup, 0},
        {INTEGER, INTEGER, INTEGER, TupleGroup, 0},
        0
    };
}

void __test__fncall(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
    
    static char * line[] = {
        "foo()",
        "(foo())",
        "(foo)()()",
        "bar(1, 2)",
        "foo(1+2*3, 4)",
        "foo((1, 2))",
        "foo((1, 2), 3, 4)",
        "foo(bar(1, 2), 3, 4)",
        "foo(1, 2, 3, bar(4, 5))",
        "foo(bar(1, (2 * 2)), 3, 4)",

    };

    static enum Lexicon check_list[][16] = {
        {WORD, Apply, 0},
        {WORD, Apply, 0},
        {WORD, Apply, Apply, 0},

        {WORD, INTEGER, INTEGER, Apply, 0},
        {WORD, INTEGER, INTEGER, INTEGER, MUL, ADD, INTEGER, Apply, 0},
        {WORD, INTEGER, INTEGER, TupleGroup, Apply, 0},
        {WORD, INTEGER, INTEGER, TupleGroup, INTEGER, INTEGER, Apply, 0},
        {WORD, WORD, INTEGER, INTEGER, Apply, INTEGER, INTEGER, Apply, 0},
        {WORD, INTEGER, INTEGER, INTEGER, WORD, INTEGER, INTEGER, Apply, 0},
        {WORD, WORD, INTEGER, INTEGER, INTEGER, MUL, Apply, INTEGER, INTEGER, Apply, 0},        
        0
    };

    for (usize i=0 ;; i++) {
        if (check_list[i][0] == 0)
            break;
        ntokens=0;
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "%s:%d failed tokenizing", __FILE__, __LINE__);
        CuAssert(tc, msg, tokenize(line[i], tokens, &ntokens, 32, NULL) == 0);
        
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on parsing expr (idx): %ld", i);
        CuAssert(tc, msg, parse_expr(line[i], tokens, ntokens, &state, ret) == 0); 

        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on index %ld", i);
        AssertTokensByRef(tc, line[i], msg, state.debug.base, check_list[i]);
        reset_state(&state);
    }
}

void __test__list_collection(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
   
    static char * line[] = {
        "[]",
        "[1]",
        "[1, 2]",
        "[1, 2+3]",
        "[1, [2+3]]",
        "[[1, 2+3]]",
        "[1, [2+3]]",  
        "[1, 2+3]+4",      
        "[][2]",
        "[][]",
        "[[]]",
    };

    static enum Lexicon check_list[][8] = {
        {TupleGroup, 0},
        {INTEGER, 0},
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, TupleGroup, 0},
        {INTEGER, INTEGER, INTEGER, TupleGroup, TupleGroup, 0},
        {INTEGER, INTEGER,  TupleGroup, INTEGER, TupleGroup, 0},
        {INTEGER, INTEGER, INTEGER, TupleGroup, 0},
        {INTEGER, INTEGER, INTEGER, TupleGroup, 0},
        0
    };
}

void __test__index_operation(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
   
    static char * line[] = {
        "[1, 2][3]",
        "[1, 2][3:4]",
        "[1, 2][:3]",
        "[1, 2][3:]",
        "[1, 2][:]",
        "[1, 2][3:4:5]",
        "[1, 2][::3]",
        "[1, 2][3::]",
        "[1, 2][3::4]",
        "word[foo::]",
        "word[foo(1, 2, 3)::]",
        "word[{2:3}[2]::]",
        "word[{2:3}[2]:]",
        "word[[1, 2, 3][4]::]",
        "word[::[1, 2, 3][4]]",
        "fn()[1]",
        "fn()[1:2:3]",
        "fn()[1::]",
        "(1, 2, 3)[1::2]",
        "{foo:33}[1]",
        "\"123\"[4]",
    };
}

void __test__set_collection(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
   
    static char * line[] = {
        "{1, 2}",
        "{1, 2, {3, 2}}",
    };
}

void __test__map_collection(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
   
    static char * line[] = {
        "{1}", // expression, not collection
        "{1: 2}",
        "{1+2:{1:3+4}}",
        "{1:2, 3:4}",
        "{1:2+3, 3:4+5}",
        "{(1, 2, 3):(4, 5, 6), 3:4+5}",
    };
}

void __test__code_block(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
   
    static char * line[] = {
        "{}",
        "{1}", // expression, not collection
        "{1}", // expression, not collection
        "{ foo(); }",
        "a = {1; 2; 3;};",
    };
}

void __test__simple_order_precedence(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
   
    static char * line[] = {
        "(1 + 3) * 4",
        "1 + 2",
        "1 + 3 * 4",
        "1 / 2 + 2",
        "a + b - c * d",
        "1 * 2 + 3",
        "1 + 2 * 3",
        "(1 + 2)",
        "((1 + 2))",
        "(1 + 3) * 4",
        "1 / (2 + 2)",
        "a + (b - c) * d",
        "1 * (2 + 3)",
        "foo.attr + 4",
        "4 + foo.attr",
        "foo.bar.attr + 2",
        "2 + foo.bar.attr",
        "x = 2",
        "x = (z = y)",
        "x = y = z",
        "x = (y = 2 * x)",
        "x = y * 2",
        "x.y.z = a.b * 2",
        0
    };

    static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD, INTEGER, MUL, 0},
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, INTEGER, MUL, ADD, 0},
        {INTEGER, INTEGER, DIV, INTEGER, ADD, 0},
        {WORD, WORD, ADD, WORD, WORD, MUL, SUB, 0},
        {INTEGER, INTEGER, MUL, INTEGER, ADD, 0},
        {INTEGER, INTEGER, INTEGER, MUL, ADD, 0},
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, ADD, INTEGER, MUL, 0},
        {INTEGER, INTEGER, INTEGER, ADD, DIV, 0},
        {WORD, WORD, WORD, SUB, WORD, MUL, ADD, 0},
        {INTEGER, INTEGER, INTEGER, ADD, MUL, 0},
        {WORD, WORD, DOT, INTEGER, ADD, 0},
        {INTEGER, WORD, WORD, DOT, ADD, 0},
        {WORD, WORD, DOT, WORD, DOT, INTEGER, ADD, 0},
        {WORD, INTEGER, EQUAL, 0},
        {WORD, WORD, WORD, EQUAL, EQUAL, 0},
        {WORD, WORD, WORD, EQUAL, EQUAL, 0},
        {WORD, WORD, INTEGER, WORD, MUL, EQUAL, EQUAL, 0},
        {WORD, WORD, INTEGER, MUL, WORD, EQUAL, EQUAL, 0},
        {WORD, WORD, DOT, WORD, DOT, WORD, WORD, DOT, INTEGER, MUL, EQUAL,  0},
        0
    };

    for (usize i=0 ;; i++) {
        if (check_list[i][0] == 0)
            break;
        ntokens=0;
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "%s:%d failed tokenizing", __FILE__, __LINE__);
        CuAssert(tc, msg, tokenize(line[i], tokens, &ntokens, 32, NULL) == 0);
        
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on parsing expr (idx): %ld", i);
        CuAssert(tc, msg, parse_expr(line[i], tokens, ntokens, &state, ret) == 0); 

        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on index %ld", i);
        AssertTokensByRef(tc, line[i], msg, state.debug.base, check_list[i]);
        reset_state(&state);
    }
}

void __test__effectively_empty_group(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
    
    static char * line[] = {
        "()",
        "(())",
        "(())()",
        "(())(())",
        "(())[:]",
        "{}",
        "{{}}",
        "{{}}()",
    };

    static enum Lexicon check_list[][16] = {
        {WORD, Apply, 0},
        {WORD, Apply, 0},
        {WORD, Apply, Apply, 0},

        {WORD, INTEGER, INTEGER, Apply, 0},
        {WORD, INTEGER, INTEGER, INTEGER, MUL, ADD, INTEGER, Apply, 0},
        {WORD, INTEGER, INTEGER, TupleGroup, Apply, 0},
        {WORD, INTEGER, INTEGER, TupleGroup, INTEGER, INTEGER, Apply, 0},
        {WORD, WORD, INTEGER, INTEGER, Apply, INTEGER, INTEGER, Apply, 0},
        {WORD, INTEGER, INTEGER, INTEGER, WORD, INTEGER, INTEGER, Apply, 0},
        {WORD, WORD, INTEGER, INTEGER, INTEGER, MUL, Apply, INTEGER, INTEGER, Apply, 0},        
        0
    };

    for (usize i=0 ;; i++) {
        if (check_list[i][0] == 0)
            break;
        ntokens=0;
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "%s:%d failed tokenizing", __FILE__, __LINE__);
        CuAssert(tc, msg, tokenize(line[i], tokens, &ntokens, 32, NULL) == 0);
        
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on parsing expr (idx): %ld", i);
        CuAssert(tc, msg, parse_expr(line[i], tokens, ntokens, &state, ret) == 0); 

        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on index %ld", i);
        AssertTokensByRef(tc, line[i], msg, state.debug.base, check_list[i]);
        reset_state(&state);
    }
}
void __test__you_know_too_much(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    usize ntokens=0;
   
    static char * line[] = {
        "()()",
        "a[:][:]",
        "(1,2,3)()",
        "(1)()",
        "().foo",
        "()+3",
        0  
    };
}


// void __test__order_precedence_right_assoc(CuTest* tc) {
//     static enum Lexicon answer[] = {INTEGER, INTEGER, INTEGER, POW, POW};
//     static char * words[] = {"3", "2", "1", "^", "^", 0};

//     usize ntokens=0,
//         nqueue=0;

//     struct Token tokens[32],
//         *queue[32],
//         *masks[2];

//     static char * line = "1 ^ 2 ^ 3"; /* 8 */

//     CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
//     CuAssertTrue(tc, ntokens == 5);

//     nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, 2);
//     CuAssertTrue(tc, __check_tokens_by_ref(queue, answer));
// }

// void __test__order_precedence_not_op(CuTest* tc) {
//     usize ntokens=0,
//         nqueue=0;
//     static enum Lexicon answers[] = {NOT, WORD, WORD, ADD, NOT, WORD, SUB};

//     struct Token tokens[32],
//         *queue[32],
//         *masks[2];

//     static char * line = "!a + b - !c";
//     CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
//     CuAssertTrue(tc, ntokens == 7);

//     nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, 2);
    
//     for (usize i=0; nqueue > i; i++) {
//         CuAssertTrue(tc, queue[i]->type == answers[i]);
//     }
// }

// void __test__order_precedense_with_fncall(CuTest* tc) {
//     usize ntokens, nqueue;
    
//     struct Token tokens[32],
//         *queue[32],
//         *masks[2];
    
//     static char * line = "first().middle.last()";
//     CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
//     CuAssertTrue(tc, ntokens == 9);

//     postfix_expr(tokens, ntokens, queue, 32, masks, 2);
// }


CuSuite* PostFixUnitTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    
    SUITE_ADD_TEST(suite, __test__simple_order_precedence);
    SUITE_ADD_TEST(suite, __test__tuple_collection);
    SUITE_ADD_TEST(suite, __test__list_collection);
    SUITE_ADD_TEST(suite, __test__set_collection);
    SUITE_ADD_TEST(suite, __test__map_collection);
    SUITE_ADD_TEST(suite, __test__fncall);
    SUITE_ADD_TEST(suite, __test__index_operation);


    return suite;
}
