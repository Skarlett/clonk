#include <string.h>
#include <stdio.h>
#include "../src/utils/vec.h"
#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/helpers.h"
#include "../src/parser/lexer/debug.h"
#include "../src/parser/expr/expr.h"
#include "../src/parser/expr/debug.h"
#include "../src/prelude.h"
#include "../src/parser/error.h"
#include "testutils.h"
#include "CuTest.h"

void __test__bug_fix(CuTest* tc) {
    struct Token tokens[32];
    usize ctr = 0, ret=0;
    char msg[64];
    memset(msg, 0, sizeof(char[64]));
    ret = tokenize("1234 - 1234", tokens, &ctr, 32, NULL);
    sprintf_token_slice(tokens, ret, msg, 64);
    printf("%s len: %ld\n", msg, ctr);
}


CuSuite* BugFix(void) {
	CuSuite* suite = CuSuiteNew();
	SUITE_ADD_TEST(suite, __test__bug_fix);
    return suite;
}

void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, BugFix());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}

int main(void)
{
	RunAllTests();
}
