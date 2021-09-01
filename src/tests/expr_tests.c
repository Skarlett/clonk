#include <string.h>
#include <stdio.h>
#include "../parser/lexer/lexer.h"
#include "../parser/lexer/helpers.h"
#include "../parser/expr/expr.h"
#include "../parser/expr/debug.h"
#include "../prelude.h"
#include "../CuTest.h"


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


// void __test__double_perthensis_1(CuTest* tc) {
//     static char * line = "((1))";
//     struct Token tokens[16];
//     struct Expr expr[2];
//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     ntokens = tokenize(line, tokens, 0);
    
//     CuAssertTrue(tc, ntokens == 1);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[1]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[0], &expr[1]) == 1);
// }

// void __test__double_perthensis_2(CuTest* tc) {
//     static char * line = "((1)) + 2";
//     struct Token tokens[16];
//     struct Expr expr[8];
//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);
//     build_int_value_T(&expr[1], 2);
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 7);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }

// void __test__double_perthensis_3(CuTest* tc) {
//     static char * line = "((1)) + ((2))";
//     struct Token tokens[16];
//     struct Expr expr[8];

//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
    
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }

// void __test__double_perthensis_4(CuTest* tc) {
//     static char * line = "(1 + (2))";
//     struct Token tokens[16];
//     struct Expr expr[8];
//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
    
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     ntokens = tokenize(line, tokens, 0);
//     CuAssertTrue(tc, ntokens == 7);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }

// void __test__double_perthensis_5(CuTest* tc) {
//     static char * line = "((1) + 2)";
//     struct Token tokens[16];
//     struct Expr expr[8];
//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
    
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     ntokens = tokenize(line, tokens, 0);
//     CuAssertTrue(tc, ntokens == 7);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    
//     ptree(&expr[2]);
//     ptree(&expr[3]);

//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
    
// }

// void __test__double_perthensis_6(CuTest* tc) {
//     static char * line = "((1) + (2))";
//     struct Token tokens[16];
//     struct Expr expr[8];
//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
    
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     ntokens = tokenize(line, tokens, 0);
//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }


// void __test__double_perthensis_7(CuTest* tc) {
//     static char * line = "((1 + 2))";
//     struct Token tokens[16];
//     struct Expr expr[8];

//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
    
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 7);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }

// void __test__double_perthensis_8(CuTest* tc) {
//     static char * line = "((1) + 2) + 3";
//     struct Token tokens[16];
//     struct Expr expr[8];

//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);

//     build_int_value_T(&expr[2], 3);

//     build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    
//     build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
//     build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);
    
//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);
// }


// void __test__double_perthensis_9(CuTest* tc) {
//     static char * line = "1 + (2 + (3))";
//     struct Token tokens[16];
//     struct Expr expr[8];

//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);

//     build_int_value_T(&expr[2], 3);

//     build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);

//     build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);

//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);
// }

// void __test__double_perthensis_10(CuTest* tc) {
//     static char * line = "1 + ((2) + 3)";
//     struct Token tokens[16];
//     struct Expr expr[8];

//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
    
//     build_int_value_T(&expr[2], 3);

//     build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    
//     build_bin_expr_T(&expr[4], &expr[2], &expr[3], Add);
    
//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[3], &expr[5]) == 1);
// }

// void __test__double_perthensis_11(CuTest* tc) {
//     static char * line = "((1 + 2 + 3))";
//     struct Token tokens[16];
//     struct Expr expr[8];

//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);
//     build_int_value_T(&expr[1], 2);
//     build_int_value_T(&expr[2], 3);

//     build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    
//     build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);
    
//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[3], &expr[5]) == 1);
// }

// void __test__double_perthensis_12(CuTest* tc) {
//     static char * line = "(1 + (2) + 3)";
//     struct Token tokens[16];
//     struct Expr expr[8];
//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);

//     build_int_value_T(&expr[2], 2);
    
//     build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    
//     build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);

//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);
// }


// void __test__double_perthensis_13(CuTest* tc) {
//     static char * line = "1 + ((2 + 3))";
//     struct Token tokens[16];
//     struct Expr expr[8];
//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);
//     build_int_value_T(&expr[1], 2);
//     build_int_value_T(&expr[2], 3);
    
    
//     build_bin_expr_T(&expr[3], &expr[1], &expr[2], Add);
//     build_bin_expr_T(&expr[4], &expr[0], &expr[3], Add);
    
//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[3], &expr[5]) == 1);
// }


// void __test__double_perthensis_14(CuTest* tc) {
//     static char * line = "((1 + 2)) + 3";
//     struct Token tokens[16];
//     struct Expr expr[8];
//     usize ntokens;
    
//     for (int i=0; 3 > i; i++) {
//         build_int_value_T(&expr[i], i+1);
//     }


//     build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
//     build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);

//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);
// }

// void __test__double_perthensis_15(CuTest* tc) {
//     static char * line = "((1 + 2)) + (( 3 + 4))";
//     struct Token tokens[16];
//     struct Expr expr[8];
//     usize ntokens;

//     for (int i=0; 4 > i; i++) {
//         build_int_value_T(&expr[i], i+1);
//     }

//     build_bin_expr_T(&expr[5], &expr[0], &expr[1], Add);

//     build_bin_expr_T(&expr[6], &expr[2], &expr[3], Add);
    
//     build_bin_expr_T(&expr[7], &expr[5], &expr[6], Add);    
    
//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[8]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[7], &expr[8]) == 1);
// }


// void __test__double_perthensis_16(CuTest* tc) {
//     static char * line = "((1 + 2) + (3 + 4))";
//     struct Token tokens[16];
//     struct Expr expr[8];

//     usize ntokens;

//     for (int i=0; 4 > i; i++) {
//         build_int_value_T(&expr[i], i+1);
//     }

//     build_bin_expr_T(&expr[5], &expr[0], &expr[1], Add);

//     build_bin_expr_T(&expr[6], &expr[2], &expr[3], Add);

//     build_bin_expr_T(&expr[7], &expr[5], &expr[6], Add);

//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[8]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[7], &expr[8]) == 1);
// }


// void __test__double_perthensis_17(CuTest* tc) {
//     static char * line = "(( ((1 + 2)) + (3 + 4) ))";
//     struct Token tokens[16];
//     struct Expr expr[8];

//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
    
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     expr[3].type = BinExprT;
//     expr[3].inner.bin.op = Add;
//     expr[3].inner.bin.lhs = &expr[2];
    
//     build_int_value_T(&expr[4], 3);
//     expr[3].inner.bin.rhs = &expr[4];

//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 9);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[3], &expr[5]) == 1);
// }


// void __test__sanity_test_1(CuTest* tc) {
//     static char * line = "1";
//     usize ntokens;
//     struct Token tokens[16];
//     struct Expr expr[2];
    
//     build_int_value_T(&expr[0], 1);

//     ntokens = tokenize(line, tokens, 0);
    
//     CuAssertTrue(tc, ntokens == 1);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[1]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[0], &expr[1]) == 1);
// }


// void __test__sanity_1_plus_1(CuTest* tc)
// {
//     static char * line = "1 + 1";
//     static enum Lexicon symbols[] = {INTEGER, ADD, INTEGER};
//     struct Token tokens[8];
//     char msg[256];

//     struct Expr expr;    
//     struct Expr answer;
//     struct Expr answer_expr[2];
    
//     for (int i=0; 2 > i; i++)
//         build_int_value_T(&answer_expr[i], i+1);

//     answer.type=BinExprT,
//     answer.inner.bin.lhs = &answer_expr[0];
//     answer.inner.bin.rhs = &answer_expr[1];
//     answer.inner.bin.op = Add;

//     usize ntokens = tokenize(line, tokens, 0);

//     sprintf(msg, "Expected [int, plus, int], instead got '%s %s %s'",
//         ptoken(tokens[0].token),
//         ptoken(tokens[1].token),
//         ptoken(tokens[2].token)
//     );

// 	CuAssertTrue(tc, ntokens == 3);
// 	CuAssert(
//         tc, 
//         msg,
//         __check_tokens(tokens, symbols, 3) == 0
//     );

//     memset(msg, 0, 255);

//     int ret = construct_expr(line, tokens, 3, &expr) == 0;
//     if (ret == 0) {
//         printf("Expected: \n");
//         ptree(&answer);
//         printf("got: \n");
//         ptree(&expr);
//     }
//     CuAssertTrue(tc, ret == 1);
// }

// void __test__1_plus_2(CuTest* tc) {
//     static char * line = "1 + 2";
//     struct Expr expr[8];
//     struct Token tokens[16];
//     usize ntokens;


//     build_int_value_T(&expr[0], 1);
//     build_int_value_T(&expr[1], 2);
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);

//     ntokens = tokenize(line, tokens, 0);
    
//     CuAssertTrue(tc, ntokens == 3);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }

// int __test__single_perthensis_1(CuTest* tc) {
//     struct Expr expr[8];
//     struct Token tokens[16];
//     static char * line="(1) + 2";        // 5
//     usize ntokens;

//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);

//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 5);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }

// void __test__single_perthensis_2(CuTest* tc) {
//     struct Expr expr[8];
//     struct Token tokens[32];
//     char *line = "1 + (2)";        // 5
//     usize ntokens;

//     build_int_value_T(&expr[0], 1);
//     build_int_value_T(&expr[1], 2);
    
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     ntokens = tokenize(line, tokens, 0);
//     CuAssertTrue(tc, ntokens == 5);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }


// void __test__single_perthensis_3(CuTest* tc) {
//     static char * line = "(1) + (2)";
    
//     static enum Lexicon check_list[] = {
//         PARAM_OPEN, INTEGER, BRACE_CLOSE, ADD, PARAM_OPEN, INTEGER, PARAM_CLOSE
//     };

//     struct Token tokens[16];
//     struct Expr expr[8];

//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
    
//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 7);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }

// void __test__single_perthensis_4(CuTest* tc) {
//     struct Expr expr[8];
//     struct Token tokens[32];
//     char *line = "(1 + 2)";        // 5
//     usize ntokens;
    
//     build_int_value_T(&expr[0], 1);
//     build_int_value_T(&expr[1], 2);

//     build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
//     ntokens = tokenize(line, tokens, 0);
//     CuAssertTrue(tc, ntokens == 5);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
// }

// void __test__single_perthensis_5(CuTest* tc) {
//     struct Expr expr[8];
//     struct Token tokens[32];
//     static char *line = "(1 + 2) + 3";    // 7
//     usize ntokens;

//     build_int_value_T(&expr[0], 1);

//     build_int_value_T(&expr[1], 2);
//     build_int_value_T(&expr[2], 3);

//     build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);

//     build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);

//     ntokens = tokenize(line, tokens, 0);
//     CuAssertTrue(tc, ntokens == 7);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);

// }

// void __test__single_perthensis_6(CuTest* tc) {
//     struct Expr expr[8];
//     struct Token tokens[32];
//     static char *line = "1 + (2 + 3)";    // 7
//     usize ntokens;

//     build_int_value_T(&expr[0], 1);
//     build_int_value_T(&expr[1], 2);

//     build_int_value_T(&expr[2], 3);

//     build_bin_expr_T(&expr[3], &expr[1], &expr[2], Add);

//     build_bin_expr_T(&expr[4], &expr[1], &expr[3], Add);

//     ntokens = tokenize(line, tokens, 0);

//     CuAssertTrue(tc, ntokens == 7);
//     CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
//     CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);

// }

int __test__fnmasks_no_function(CuTest* tc) {
    struct Token input[16];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    
    static char *source = "1 + 2 + 3";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 5);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 0);
    CuAssertTrue(tc, output_ctr == (size_t)input_sz);

    for (int i=0; input_sz > i; i++)
        // assert memory addresses are the same
        CuAssertTrue(tc, &input[i] == output[i]);
}

int __test__fnmasks_empty_function(CuTest* tc) {
    struct Token input[16];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    
    static char *source = "foo()";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 3);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->token == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 2);
}

int __test__fnmasks_with_args(CuTest* tc) {
    struct Token input[16];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    
    static char *source = "foo(a, b, c)";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 8);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->token == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 7);
}

int __test__fnmasks_with_operators_in_args(CuTest* tc) {
    struct Token input[16];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    
    static char *source = "foo(a == 2, b+2, c)";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 12);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->token == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 11);
}


int __test__fnmasks_with_parthesis_in_args(CuTest* tc) {
    struct Token input[48];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[4];
    size_t output_ctr = 0;
    
    static char *source = "foo(((a == 2)), (b+2)*5, c)";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 20);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            4,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->token == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 19);
}

int __test__fnmasks_multi_empty(CuTest* tc) {
    struct Token input[48];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    static char *source = "foo() + foo()";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 7);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 2);
    CuAssertTrue(tc, output_ctr == 3);
    
    CuAssertTrue(tc, output[0]->token == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 2);
    
    CuAssertTrue(tc, output[1]->token == ADD);
    CuAssertTrue(tc, output[1]->start == 6);
    CuAssertTrue(tc, output[1]->end == 6);

    CuAssertTrue(tc, output[2]->token == FNMASK);
    CuAssertTrue(tc, output[2]->start == 4);
    CuAssertTrue(tc, output[2]->end == 6);

    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, input[output[0]->start].token == WORD);
    CuAssertTrue(tc, input[output[0]->start].start == 0);
    CuAssertTrue(tc, input[output[0]->start].end == 2);

    CuAssertTrue(tc, input[0].token == WORD);
    CuAssertTrue(tc, input[0].start == 0);
    CuAssertTrue(tc, input[0].end == 2);

    CuAssertTrue(tc, input[1].token == PARAM_OPEN);
    CuAssertTrue(tc, input[1].start == 3);
    CuAssertTrue(tc, input[1].end == 3);

    CuAssertTrue(tc, input[2].token == PARAM_CLOSE);
    CuAssertTrue(tc, input[2].start == 4);
    CuAssertTrue(tc, input[2].end == 4);

    CuAssertTrue(tc, output[0]->end == 2);
    CuAssertTrue(tc, input[output[0]->end].token == PARAM_CLOSE);
    CuAssertTrue(tc, input[output[0]->end].start == 4);
    CuAssertTrue(tc, input[output[0]->end].end == 4);

    CuAssertTrue(tc, input[3].token == ADD);
    CuAssertTrue(tc, input[3].start == 6);
    CuAssertTrue(tc, input[3].end == 6);

    CuAssertTrue(tc, output[2]->start == 4);
    CuAssertTrue(tc, input[output[2]->start].token == WORD);
    CuAssertTrue(tc, input[output[2]->start].start == 8);
    CuAssertTrue(tc, input[output[2]->start].end == 10);

    CuAssertTrue(tc, input[4].token == WORD);
    CuAssertTrue(tc, input[4].start == 8);
    CuAssertTrue(tc, input[4].end == 10);

    CuAssertTrue(tc, input[5].token == PARAM_OPEN);
    CuAssertTrue(tc, input[5].start == 11);
    CuAssertTrue(tc, input[5].end == 11);

    CuAssertTrue(tc, input[6].token == PARAM_CLOSE);
    CuAssertTrue(tc, input[6].start == 12);
    CuAssertTrue(tc, input[6].end == 12);
    
    CuAssertTrue(tc, output[2]->end == 6);
    CuAssertTrue(tc, input[output[2]->end].token == PARAM_CLOSE);
    CuAssertTrue(tc, input[output[2]->end].start == 12);
    CuAssertTrue(tc, input[output[2]->end].end == 12);
}

int __test__fnmasks_multi_with_args(CuTest* tc) {
    struct Token input[48];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    static char *source = "foo(a, b, c) + foo(a, b, c)";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 17);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 2);
    CuAssertTrue(tc, output_ctr == 3);
    CuAssertTrue(tc, output[0]->token == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 8);
    
    CuAssertTrue(tc, output[1]->token == ADD);
    CuAssertTrue(tc, output[1]->start == 13);
    CuAssertTrue(tc, output[1]->end == 13);

    CuAssertTrue(tc, output[2]->token == FNMASK);
    CuAssertTrue(tc, output[2]->start == 9);
    CuAssertTrue(tc, output[2]->end == 17);
}

int __test__fnmasks_multi_with_operators_in_args(CuTest* tc) {
    struct Token input[48];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    static char *source = "foo(a + 2, b-2, c) + foo(a^2, b%2, c*0)";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 27);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 2);
    CuAssertTrue(tc, output_ctr == 3);
    CuAssertTrue(tc, output[0]->token == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 11);
    
    CuAssertTrue(tc, output[1]->token == ADD);
    CuAssertTrue(tc, output[1]->start == 19);
    CuAssertTrue(tc, output[1]->end == 19);

    CuAssertTrue(tc, output[2]->token == FNMASK);
    CuAssertTrue(tc, output[2]->start == 13);
    CuAssertTrue(tc, output[2]->end == 26);
}

int __test__fnmasks_multi_with_parathesis_in_args(CuTest* tc) {
    struct Token input[48];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    static char *source = "foo(((((a^2) + (b%2), c*0)))) + 1";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 26);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 3);
    CuAssertTrue(tc, output[0]->token == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 23);
    
    CuAssertTrue(tc, output[1]->token == ADD);
    CuAssertTrue(tc, output[1]->start == 30);
    CuAssertTrue(tc, output[1]->end == 30);

    CuAssertTrue(tc, output[2]->token == INTEGER);
    CuAssertTrue(tc, output[2]->start == 32);
    CuAssertTrue(tc, output[2]->end == 32);
}

int __test__fnmasks_with_unbalanced_parthesis_left_of_args(CuTest* tc) {
    struct Token input[48];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[4];
    size_t output_ctr = 0;
    
    static char *source = "foo(((a == 2), (b+2)*5, c)";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 19);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            4,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == -1
    );
}

int __test__fnmasks_with_unbalanced_parthesis_right_of_args(CuTest* tc) {
    struct Token input[48];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[4];
    size_t output_ctr = 0;
    
    static char *source = "foo((a == 2), (b+2)*5, c))";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 19);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            4,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == -1
    );
}

/* 
    The fitness pacer test 
    is multi-stage aerobic capacity test that
    progressively gets more difficult as it continues.
    the 20 meter pacer test will begin in 30 seconds,
    line up at the start.
*/
int __test__fnmasks_pacer_test(CuTest* tc) {
    struct Token input[256];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    
    static char *source = "" \
        "print(foo(a == (2), (b)+2, ((_c)))) + " \
        "foo() >= oop(foo(aa), sin(x), floor(y)), sin(x), vectorize(matrix[i][j][k]) " \
        "&& !is_left(maybe?)";        
   

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 12);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    
}
void __test__order_precedence(CuTest* tc) {
    int ntokens, nqueue;
    
    struct Token tokens[32],
        *queue[32],
        *masks[2];
    
    static char * line[] = {
        "1 + 2",
        "1 + 3 * 4",
        "1/2 + 2",
        "(a-2)*3",
        "foo(a, n)+b",
    };
    
    static int sz[] = {
        3, 5, 5, 7, 3
    };

    static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
        {INTEGER, INTEGER, INTEGER, DIV, ADD},
        {WORD, INTEGER, SUB, INTEGER, INTEGER, MUL},
        {FNMASK, ADD, WORD},
    };

    for (int i=0; 5 > i; i++) {
        ntokens = tokenize(line[i], tokens, 0);
        nqueue = construct_postfix_queue(tokens, ntokens, queue, 32, masks, 2);
    }

    CuAssertTrue(tc, 2 == 1);
}


int __test__fnmasks_with_application(CuTest* tc) {
    struct Token input[48];
    int input_sz = 0;
    struct Token masks[4];
    size_t masks_ctr = 0;
    struct Token *output[16];
    size_t output_ctr = 0;
    static char *source = "foo().length + 1";

    input_sz = tokenize(source, input, 0);
    CuAssertTrue(tc, input_sz == 26);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            (size_t)input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 3);
    CuAssertTrue(tc, output[0]->token == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 2);
    
    CuAssertTrue(tc, output[1]->token == DOT);
    CuAssertTrue(tc, output[1]->start == 5);
    CuAssertTrue(tc, output[1]->end == 5);

    CuAssertTrue(tc, output[2]->token == WORD);
    CuAssertTrue(tc, output[2]->start == 6);
    CuAssertTrue(tc, output[2]->end == 11);

    CuAssertTrue(tc, output[3]->token == ADD);
    CuAssertTrue(tc, output[3]->start == 13);
    CuAssertTrue(tc, output[3]->end == 13);

    CuAssertTrue(tc, output[4]->token == INTEGER);
    CuAssertTrue(tc, output[4]->start == 15);
    CuAssertTrue(tc, output[4]->end == 15);
}


CuSuite* ExprUnitTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__fnmasks_no_function);
    SUITE_ADD_TEST(suite, __test__fnmasks_empty_function);
    SUITE_ADD_TEST(suite, __test__fnmasks_with_args);
    SUITE_ADD_TEST(suite, __test__fnmasks_with_operators_in_args);
    SUITE_ADD_TEST(suite, __test__fnmasks_with_parthesis_in_args);
    
    SUITE_ADD_TEST(suite, __test__fnmasks_multi_empty);
    SUITE_ADD_TEST(suite, __test__fnmasks_multi_with_operators_in_args);
    SUITE_ADD_TEST(suite, __test__fnmasks_multi_with_parathesis_in_args);

    SUITE_ADD_TEST(suite, __test__fnmasks_with_unbalanced_parthesis_left_of_args);
    SUITE_ADD_TEST(suite, __test__fnmasks_with_unbalanced_parthesis_right_of_args);

    SUITE_ADD_TEST(suite, __test__fnmasks_with_application);
    
    //SUITE_ADD_TEST(suite, __test__order_precedence);

    // SUITE_ADD_TEST(suite, __test__sanity_expr_cmp);
	// SUITE_ADD_TEST(suite, __test__sanity_test_1);
	// SUITE_ADD_TEST(suite, __test__sanity_1_plus_1);
	// SUITE_ADD_TEST(suite, __test__1_plus_2);

	// SUITE_ADD_TEST(suite, __test__single_perthensis_1);
	// SUITE_ADD_TEST(suite, __test__single_perthensis_2);
    // SUITE_ADD_TEST(suite, __test__single_perthensis_3);
    // SUITE_ADD_TEST(suite, __test__single_perthensis_4);
    // SUITE_ADD_TEST(suite, __test__single_perthensis_5);
    // SUITE_ADD_TEST(suite, __test__single_perthensis_6);
	// SUITE_ADD_TEST(suite, __test__double_perthensis_1);
	// SUITE_ADD_TEST(suite, __test__double_perthensis_2);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_3);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_4);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_5);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_6);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_7);
	// SUITE_ADD_TEST(suite, __test__double_perthensis_8);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_9);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_10);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_11);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_12);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_13);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_14);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_15);
	// SUITE_ADD_TEST(suite, __test__double_perthensis_16);
    // SUITE_ADD_TEST(suite, __test__double_perthensis_17);

    return suite;
}
