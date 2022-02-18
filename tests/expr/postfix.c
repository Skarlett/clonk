#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "../../src/utils/vec.h"
#include "../../src/parser/lexer/lexer.h"
#include "../../src/parser/expr/expr.h"
#include "../../src/prelude.h"
#include "../testutils.h"
#include "../CuTest.h"
#include "common.h"

int8_t into_ref_array(
    struct onk_token_t input[],
    struct onk_token_t *out[],
    const uint16_t input_sz,
    const uint16_t out_sz)
{
    if (input_sz > out_sz)
        return -1;
    
    for (uint16_t i=0; input_sz > i; i++)
        out[i] = &input[i];

    return 0;
}

void __test__effectively_empty_group(CuTest* tc)
{
    struct onk_token_t tokens[32];
    struct Parser state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
    
    static char * src_code[] = {
        "()",
        "(())",
        "(())()",
        "(())(())",
        "(())[:]",
        "{}",
        "{{}}",
        "{{}}()",
    };

}
void __test__you_know_too_much(CuTest* tc)
{
    struct onk_token_t tokens[32];
    struct Parser state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
   
    static char * src_code[] = {
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
//     static enum onk_lexicon_t answer[] = {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_POW_TOKEN, ONK_POW_TOKEN};
//     static char * words[] = {"3", "2", "1", "^", "^", 0};

//     uint16_t ntokens=0,
//         nqueue=0;

//     struct onk_token_t tokens[32],
//         *queue[32],
//         *masks[2];

//     static char * src_code = "1 ^ 2 ^ 3"; /* 8 */

//     CuAssertTrue(tc, onk_tokenize(src_code, tokens, &ntokens, NULL) == 0);
//     CuAssertTrue(tc, ntokens == 5);
//     nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, 2);
//     CuAssertTrue(tc, __check_tokens_by_ref(queue, answer));
// }



// void __test__order_precedence_not_op(CuTest* tc) {
//     uint16_t ntokens=0,
//         nqueue=0;
//     static enum onk_lexicon_t answers[] = {ONK_NOT_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_ADD_TOKEN, ONK_NOT_TOKEN, ONK_WORD_TOKEN, ONK_SUB_TOKEN};
//     struct onk_token_t tokens[32],
//         *queue[32],
//         *masks[2];
//     static char * src_code = "!a + b - !c";
//     CuAssertTrue(tc, onk_tokenize(src_code, tokens, &ntokens, NULL) == 0);
//     CuAssertTrue(tc, ntokens == 7);
//     nqueue = postfix_expr(tokens, ntokens, queue, 32, masks, 2);
//     for (uint16_t i=0; nqueue > i; i++) {
//         CuAssertTrue(tc, queue[i]->type == answers[i]);
//     }
// }



// CuSuite* PostFixUnitTestSuite(void) {
// 	CuSuite* suite = CuSuiteNew();
//     return suite;
// }
