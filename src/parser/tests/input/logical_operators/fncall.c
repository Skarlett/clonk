
#include "../CuTest.h"
#include "common.h"
#include "../../src/utils/vec.h"
#include "../../src/parser/lexer/lexer.h"
#include "../../src/parser/expr/expr.h"
#include "../../src/prelude.h"
#include "../CuTest.h"
// void __test__order_precedense_with_fncall(CuTest* tc) {
//     uint16_t ntokens, nqueue;
    
//     struct onk_token_t tokens[32],
//         *queue[32],
//         *masks[2];
    
//     static char * src_code = "first().middle.last()";
//     CuAssertTrue(tc, onk_tokenize(src_code, tokens, &ntokens, NULL) == 0);
//     CuAssertTrue(tc, ntokens == 9);

//     postfix_expr(tokens, ntokens, queue, 32, masks, 2);
// }


void __test__fncall(CuTest* tc) {
    struct onk_token_t tokens[32];
    struct onk_parser_state_tstate;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
    
    static char * src_code[] = {
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

    static enum onk_lexicon_t check_list[][16] = {
        {ONK_WORD_TOKEN, onk_apply_op_token, 0},
        {ONK_WORD_TOKEN, onk_apply_op_token, 0},
        {ONK_WORD_TOKEN, onk_apply_op_token, onk_apply_op_token, 0},

        {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_apply_op_token, 0},
        {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, onk_apply_op_token, 0},
        {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_tuple_group_token, onk_apply_op_token, 0},
        {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_tuple_group_token, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_apply_op_token, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_apply_op_token, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_apply_op_token, 0},
        {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_apply_op_token, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, onk_apply_op_token, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_apply_op_token, 0},        
        0
    };

    for (uint16_t i=0 ;; i++) {
        if (check_list[i][0] == 0)
            break;
        ntokens=0;
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "%s:%d failed tokenizing", __FILE__, __SRC_CODE__);
        CuAssert(tc, msg, onk_tokenize(src_code[i], tokens, &ntokens, 32, false, NULL) == 0);
        
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on parsing expr (idx): %d", i);
        CuAssert(tc, msg, parse_expr(src_code[i], tokens, ntokens, &state, ret) == 0); 

        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on index %ld", i);
        onk_assert_tokens_by_ref(tc, src_code[i], msg, state.debug.base, check_list[i]);
        parser_reset(&state);
    }
}

 // CuSuite* PostFixUnitTestSuite(void) {
 // 	CuSuite* suite = CuSuiteNew();
-//     SUITE_ADD_TEST(suite, __test__simple_order_precedence);
-//     SUITE_ADD_TEST(suite, __test__tuple_collection);
-//     SUITE_ADD_TEST(suite, __test__list_collection);
-//     SUITE_ADD_TEST(suite, __test__set_collection);
-//     SUITE_ADD_TEST(suite, __test__map_collection);
-//     SUITE_ADD_TEST(suite, __test__fncall);
-//     SUITE_ADD_TEST(suite, __test__index_operation);
 //     return suite;
 // }
