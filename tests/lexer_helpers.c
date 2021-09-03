#include "../src/prelude.h"
#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/helpers.h"

#include "CuTest.h"

void __test__is_balanced(CuTest* tc)
{
    usize sz=0;
    struct Token tokens[32];

    int answers[] = {
        1, 1, 1, 1,
        1, 1, 1, 
        0, 0, 0
    };

    static char * line[] = {
        "1 + 2",
        "1 + (2)",
        "(1) + 2",
        "(1 + 2)",
        "foo(1 + 2)",
        "(1 + 2) + 2",
        "(1 + 2) + foo()",
        ")(1+2)",
        "(1+2",
        "9)",
        0
    };
    
    for (int i=0; 10 > i; i++) {
        CuAssertTrue(tc, tokenize(line[i], tokens, &sz, NULL) == 0);
        CuAssertTrue(tc, is_balanced(tokens, sz) == answers[i]);
        sz=0;
    }
}


CuSuite* LexerHelpersUnitTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, __test__is_balanced);
    
    return suite;
}