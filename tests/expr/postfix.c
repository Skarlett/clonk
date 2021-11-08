#include <bits/types/__FILE.h>
#include <string.h>
#include <stdio.h>
#include "../../src/utils/vec.h"
#include "../../src/parser/lexer/lexer.h"
#include "../../src/parser/lexer/helpers.h"
#include "../../src/parser/expr/expr.h"
#include "../../src/parser/expr/debug.h"
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

void __test__simple_order_precedence(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[128];
    usize ntokens=0;
   
    static char * line[] = {
        "1 + 2",
        "1 + 3 * 4",
        "1 / 2 + 2",
        "a + b - c * d",
        "1 * 2 + 3",
        "1 + 2 * 3",
        "(1 + 2)",
        "(1 + 3) * 4",
        "1 / (2 + 2)",
        "a + (b - c) * d",
        "1 * (2 + 3)",
        "(1 + 2) * 3",
        "((1 + 2))",
        "(1 + 3) * 4",
        "1 / (2 + 2)",
        "a + (b - c) * d",
        "1 * (2 + 3)",
        "(1 + 2) * 3",
        "foo.attr + 4",
        "4 + foo.attr",
        "foo.bar.attr + 2",
        "2 + foo.bar.attr",
        0
    };

    static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
        {INTEGER, INTEGER, DIV, INTEGER, ADD},
        {WORD, WORD, ADD, WORD, WORD, MUL, SUB},
        {INTEGER, INTEGER, MUL, INTEGER, ADD},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
        {INTEGER, INTEGER, ADD},
        {INTEGER, INTEGER, ADD, INTEGER, MUL},
        {INTEGER, INTEGER, INTEGER, ADD, DIV},
        {WORD, WORD, WORD, SUB, WORD, MUL, ADD},
        {INTEGER, INTEGER, INTEGER, ADD, MUL},
        {INTEGER, INTEGER, ADD, INTEGER, MUL},
        {INTEGER, INTEGER, ADD},
        {INTEGER, INTEGER, ADD, INTEGER, MUL},
        {INTEGER, INTEGER, INTEGER, ADD, DIV},
        {WORD, WORD, WORD, SUB, WORD, MUL, ADD},
        {INTEGER, INTEGER, INTEGER, ADD, MUL},
        {INTEGER, INTEGER, ADD, INTEGER, MUL},
        {WORD, WORD, DOT, INTEGER, ADD},
        {INTEGER, WORD, WORD, DOT, ADD},
        {WORD, WORD, WORD, DOT, DOT, INTEGER, ADD}
    };

    for (usize i=0; 21 > i; i++) {
        ntokens=0;
        memset(msg, 0, 128);
        sprintf(msg, "%s:%d failed tokenizing", __FILE__, __LINE__);
        CuAssert(tc, msg, tokenize(line[i], tokens, &ntokens, 32, NULL) == 0);
        
        memset(msg, 0, 128);
        sprintf(msg, "failed on parsing expr (idx): %ld", i);
        CuAssert(tc, msg, parse_expr(line[i], tokens, ntokens, &state, ret) == 0); 
        
        memset(msg, 0, 128);
        sprintf(msg, "failed on index %ld", i);
        CuAssert(tc, msg, seq_eql_ty(state.debug.base, check_list[i], state.debug.len) == 0);
        reset_state(&state);
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

    return suite;
}
