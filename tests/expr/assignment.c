
#include "../CuTest.h"
#include "common.h"
#include "../../src/utils/vec.h"
#include "../../src/parser/lexer/lexer.h"
#include "../../src/parser/expr/expr.h"
#include "../../src/prelude.h"
//#include "../../src/parser/error.h"
#include "../testutils.h"
#include "../CuTest.h"

void __test__set_collection(CuTest* tc) {
    struct Token tokens[32];
    struct Parser state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
   
    static char * src_code[] = {
        "x = 10",
        "x[2] = 10",
        "x['123'] = 123"

        // should be error
        "[2, 2][0] = 1"
    };
}