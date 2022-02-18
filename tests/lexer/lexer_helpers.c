#include <stdint.h>
#include <stdio.h>
#include "../../src/prelude.h"
#include "../../src/parser/lexer/lexer.h"
#include "../CuTest.h"

void __test__is_balanced(CuTest* tc)
{
    uint16_t sz=0;
    char msg[64];
    struct onk_token_t tokens[32];

    int8_t answers[] = {
        1, 1, 1, 1,
        1, 1, 1, 1, 
        0, 0, 0, 0,
        0, 0, 1, 1
    };

    static char * src_code[] = {
        "1 + 2",
        "1 + (2)",
        "(1) + 2",
        "(1 + 2)",
        "foo(1 + 2)",
        "(1 + 2) + 2",
        "(1 + 2) + foo()",
        "a[1]",
        ")(1+2)",
        "(1+2",
        "9)",
        "([6]",
        "[\n]]",
        "[[\n]",
        "{__}",
        "{{}}{{}{}}{{{}}}",
        0
    };
    
    for (int i=0; 16 > i; i++) {
        CuAssertTrue(tc, onk_tokenize(src_code[i], tokens, &sz, 32, false, NULL) == 0);
        sprintf(msg, "failed on `src_code[%d]`", i);
        CuAssert(tc, msg, is_balanced(tokens, sz) == answers[i]);
        sz=0;

    }
}


CuSuite* LexerHelpersUnitTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    return suite;
}