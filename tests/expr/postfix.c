#include <string.h>
#include <stdio.h>
#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/helpers.h"
#include "../src/parser/expr/expr.h"
#include "../src/parser/expr/debug.h"
#include "../src/prelude.h"
#include "../src/parser/error.h"
#include "common.h"
#include "CuTest.h"



// /*
//    Prove incremential changes cause comparsion failure.
// */
// void __test__sanity_expr_cmp(CuTest* tc)
// {
//     struct Expr a, b;
    
//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     a.inner.uni.interal_data.symbol.tag = ValueTag;
//     a.inner.uni.interal_data.symbol.inner.value.type = IntT;
//     a.inner.uni.interal_data.symbol.inner.value.data.integer = 1;

//     CuAssertTrue(tc, cmpexpr(&a, &a) == 1);

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;
//     b.inner.uni.interal_data.symbol.tag = VariableTag;
//     b.inner.uni.interal_data.symbol.inner.variable = "a";
    
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     CuAssertTrue(tc, cmpexpr(&b, &b) == 1);

//     a.inner.uni.interal_data.symbol.tag = VariableTag;
//     a.inner.uni.interal_data.symbol.inner.variable = "b";

//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;

//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     a.inner.uni.interal_data.symbol.inner.variable = "a";
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
//     a.type = UndefinedExprT;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     b.type = UndefinedExprT;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
//     a.type = UniExprT;
//     b.type = UniExprT;


//     a.inner.uni.op = UniOpNop;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
    
//     a.inner.uni.op = UniValue;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

//     a.inner.uni.interal_data.symbol.tag = NullTag;
//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);

//     b.inner.uni.interal_data.symbol.tag = NullTag;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;

//     a.inner.uni.interal_data.symbol.tag = ValueTag;
//     b.inner.uni.interal_data.symbol.tag = ValueTag;

//     a.inner.uni.interal_data.symbol.inner.value.type = IntT;
//     b.inner.uni.interal_data.symbol.inner.value.type = IntT;
//     a.inner.uni.interal_data.symbol.inner.value.data.integer = 1;
//     b.inner.uni.interal_data.symbol.inner.value.data.integer = 2;

//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     b.inner.uni.interal_data.symbol.inner.value.data.integer = 1;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;

//     b.inner.uni.interal_data.symbol.inner.value.type = StringT;
//     b.inner.uni.interal_data.symbol.inner.value.data.string.capacity = 0;
//     b.inner.uni.interal_data.symbol.inner.value.data.string.length = 5;
//     b.inner.uni.interal_data.symbol.inner.value.data.string.ptr = "data\0";
    
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     a.inner.uni.interal_data.symbol.inner.value.type = StringT;
//     a.inner.uni.interal_data.symbol.inner.value.data.string.capacity = 0;
//     a.inner.uni.interal_data.symbol.inner.value.data.string.length = 5;
//     a.inner.uni.interal_data.symbol.inner.value.data.string.ptr = "data\0";
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
    
//     struct Expr bin;
//     bin.type = BinExprT;
//     bin.inner.bin.lhs = &a;
//     bin.inner.bin.rhs = &b;

//     CuAssertTrue(tc, cmpexpr(&bin, &bin) == 1);
//     CuAssertTrue(tc, cmpexpr(&a, &bin) == 0);

//     //TODO add test cases for fncalls
// }



void __test__order_precedence(CuTest* tc) {
    usize ntokens=0, nqueue=0;
    struct Token tokens[32],
        *queue[32],
        *masks[2];

    int mask_ctr = 0;
    
    static char * line[] = {
        "1 + 2",
        "1 + 3 * 4",
        "1 / 2 + 2",
        "(a - 2) * 3",
        "a + b - c * d",
        "1 * 2 + 3",
        0
    };
    
    char msg[64]; 

    static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
        {INTEGER, INTEGER, INTEGER, DIV, ADD},
        {WORD, INTEGER, SUB, INTEGER, INTEGER, MUL},
        {WORD, WORD, WORD, WORD, ADD, SUB, MUL},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
    };

    for (usize i=0; 6 > i; i++) {
        CuAssertTrue(tc, tokenize(line[i], tokens, &ntokens, NULL) == 0);
        sprintf(msg, "failed on index %d", i);
        nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, );
        CuAssert(tc, msg, __check_tokens_by_ref(queue, check_list[i], nqueue));
    }

}

void __test__order_precedence_left_assoc(CuTest* tc) {
    usize ntokens=0, nqueue=0;
    
    static enum Lexicon answer[][5] = {
        {INTEGER, INTEGER, INTEGER, SUB, SUB},
        {INTEGER, INTEGER, INTEGER, DIV, DIV}
    };
    
    static char * words[][12] = {
        {"3", "2", "1", "-", "-", 0},
        {"12", "2", "2", "/", "/", 0},
    };

    struct Token tokens[32],
        *queue[32],
        *masks[2];

    static char * line[] = {
        "3 - 2 - 1", /* 0 */
        "12 / 2 / 3", /* 2 */
        0
    };
    for (usize i=0; 2 > i; i++) {
        CuAssertTrue(tc, tokenize(line[i], tokens, &ntokens, NULL) == 0);
        nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, 2);
        CuAssertTrue(tc, nqueue == 5);
        CuAssertTrue(tc, __check_tokens_by_ref(queue, answer[i]));

        for (usize j=0; nqueue > j; j++) {
            CuAssertTrue(tc, strncmp(
                line+queue[j]->start,
                words[j],
                queue[j]->end-queue[j]->start
            ) == 0);
        }
    }
}


void __test__order_precedence_right_assoc(CuTest* tc) {
    static enum Lexicon answer[] = {INTEGER, INTEGER, INTEGER, POW, POW};
    static char * words[] = {"3", "2", "1", "^", "^", 0};

    usize ntokens=0,
        nqueue=0;

    struct Token tokens[32],
        *queue[32],
        *masks[2];

    static char * line = "1 ^ 2 ^ 3"; /* 8 */

    CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 5);

    nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, 2);
    CuAssertTrue(tc, __check_tokens_by_ref(queue, answer));
}

void __test__order_precedence_not_op(CuTest* tc) {
    usize ntokens=0,
        nqueue=0;
    static enum Lexicon answers[] = {NOT, WORD, WORD, ADD, NOT, WORD, SUB};

    struct Token tokens[32],
        *queue[32],
        *masks[2];

    static char * line = "!a + b - !c";
    CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 7);

    nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, 2);
    
    for (usize i=0; nqueue > i; i++) {
        CuAssertTrue(tc, queue[i]->type == answers[i]);
    }
}

void __test__order_precedense_apply_op(CuTest* tc) {
    usize ntokens, nqueue;
    static enum Lexicon answer[] = {WORD, WORD, WORD, DOT, DOT};
    static char * words[] = {"last", "middle", "first", ".", ".", 0};    
    struct Token tokens[32],
        *queue[32],
        *masks[2];
    
    static char * line = "first.middle.last";
    CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 5);

    nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, 2);
    CuAssertTrue(tc, nqueue == 5);

    for (usize i=0; nqueue > i; i++) {
        CuAssertTrue(tc, queue[i]->type == answer[i]);
        CuAssertTrue(tc, strncmp(
            line+queue[i]->start,
            words[i],
            queue[i]->end-queue[i]->start
        ) == 0);
    }
}

void __test__order_precedense_with_fncall(CuTest* tc) {
    usize ntokens, nqueue;
    
    struct Token tokens[32],
        *queue[32],
        *masks[2];
    
    static char * line = "first().middle.last()";
    CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 9);

    postfix_expr(tokens, ntokens, queue, 32, masks, 2);
}


CuSuite* ExprUnitTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    
    //SUITE_ADD_TEST(suite, __test__fnmasks_with_application);
    
    // segmentation fault
    // SUITE_ADD_TEST(suite, __test__order_precedence_not_op);
    // SUITE_ADD_TEST(suite, __test__order_precedence);

    return suite;
}
