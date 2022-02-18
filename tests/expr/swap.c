
#include "../CuTest.h"
#include "common.h"
#include "../../src/utils/vec.h"
#include "../../src/parser/lexer/lexer.h"
#include "../../src/parser/expr/expr.h"
#include "../../src/prelude.h"
#include "../../src/parser/error.h"
#include "../CuTest.h"

void __test__swap_expr(CuTest* tc) {
    char * src_code = "x, y = y, x";
}

CuSuite* SwapTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    return suite;
}
