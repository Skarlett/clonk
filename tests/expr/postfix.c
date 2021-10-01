#include <string.h>
#include <stdio.h>
#include "../../src/parser/lexer/lexer.h"
#include "../../src/parser/lexer/helpers.h"
#include "../../src/parser/expr/expr.h"
#include "../../src/parser/expr/debug.h"
#include "../../src/prelude.h"
#include "../../src/parser/error.h"
#include "../common.h"
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


void __test__simple_order_precedence(CuTest* tc) {
    usize ntokens=0, nqueue=0, nout = 0;
    struct Token tokens[32],
        *queue[32],
        *output[32];
    
    static char * line[] = {
        "1 + 2",
        "1 + 3 * 4",
        "1 / 2 + 2",
        "a + b - c * d",
        "1 * 2 + 3",
        "1 + 2 * 3",
        0
    };
    
    char msg[64]; 

    static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
        {INTEGER, INTEGER, DIV, INTEGER, ADD},
        {WORD, WORD, ADD, WORD, WORD, MUL, SUB},
        {INTEGER, INTEGER, MUL, INTEGER, ADD},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},

    };

    for (usize i=0; 6 > i; i++) {
        ntokens=0;
        CuAssertTrue(tc, tokenize(line[i], tokens, &ntokens, NULL) == 0);
        CuAssertTrue(tc, into_ref_array(tokens, queue, ntokens, 32) == 0);

        sprintf(msg, "failed on index %ld", i);

        CuAssertTrue(tc, postfix_expr(queue, ntokens, output, 32, &nout, NULL) == 0);  
        
        AssertTokensByRef(tc, msg, output, check_list[i], nout);
    }
}

void __test__order_precedence(CuTest* tc) {
    usize ntokens=0, nqueue=0, nout = 0;
    struct Token tokens[32],
        *queue[32],
        *output[32];
    
    static char * line[] = {
        "(1 + 2)",
        "(1 + 3) * 4",
        "1 / (2 + 2)",
        "a + (b - c) * d",
        "1 * (2 + 3)",
        "(1 + 2) * 3",
        0
    };
    
    char msg[64]; 

    static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD},
        {INTEGER, INTEGER, ADD, INTEGER, MUL},
        {INTEGER, INTEGER, INTEGER, ADD, DIV},
        {WORD, WORD, WORD, SUB, WORD, MUL, ADD},
        {INTEGER, INTEGER, INTEGER, ADD, MUL},
        {INTEGER, INTEGER, ADD, INTEGER, MUL},
    };

    for (usize i=0; 6 > i; i++) {
        ntokens=0;
        CuAssertTrue(tc, tokenize(line[i], tokens, &ntokens, NULL) == 0);
        CuAssertTrue(tc, into_ref_array(tokens, queue, ntokens, 32) == 0);

        sprintf(msg, "failed on index %ld", i);

        CuAssertTrue(tc, postfix_expr(queue, ntokens, output, 32, &nout, NULL) == 0);  
        
        AssertTokensByRef(tc, msg, output, check_list[i], nout);
    }
}


// void __test__order_precedence_left_assoc(CuTest* tc) {
//     usize ntokens=0, nqueue=0;
    
//     static enum Lexicon answer[][5] = {
//         {INTEGER, INTEGER, INTEGER, SUB, SUB},
//         {INTEGER, INTEGER, INTEGER, DIV, DIV}
//     };
    
//     static char * words[][12] = {
//         {"3", "2", "1", "-", "-", 0},
//         {"12", "2", "2", "/", "/", 0},
//     };

//     struct Token tokens[32],
//         *queue[32],
//         *masks[2];

//     static char * line[] = {
//         "3 - 2 - 1", /* 0 */
//         "12 / 2 / 3", /* 2 */
//         0
//     };
//     for (usize i=0; 2 > i; i++) {
//         CuAssertTrue(tc, tokenize(line[i], tokens, &ntokens, NULL) == 0);

//         nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, 2);
//         CuAssertTrue(tc, nqueue == 5);
//         CuAssertTrue(tc, __check_tokens_by_ref(queue, answer[i]));

//         for (usize j=0; nqueue > j; j++) {
//             CuAssertTrue(tc, strncmp(
//                 line+queue[j]->start,
//                 words[j],
//                 queue[j]->end-queue[j]->start
//             ) == 0);
//         }
//     }
// }


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

void __test__apply_op(CuTest* tc) {
    usize ntokens=0, nout=0;
    char msg[128];
    static enum Lexicon answer[] = {WORD, WORD, DOT, WORD, DOT};
    static char * words[] = {"first", "middle", ".", "last", ".", 0};    
    struct Token tokens[32],
        *queue[32],
        *output[32];
    
    static char * line = "first.middle.last";
    CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 5);

    into_ref_array(tokens, queue, ntokens, 32);
    CuAssertTrue(tc, postfix_expr(queue, ntokens, output, 32, &nout, NULL) == 0);
    AssertTokensByRef(tc, "", output, answer, 5);
    CuAssertTrue(tc, nout == 5);
}

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
    
    SUITE_ADD_TEST(suite, __test__apply_op);
    SUITE_ADD_TEST(suite, __test__simple_order_precedence);
    SUITE_ADD_TEST(suite, __test__order_precedence);

    return suite;
}
