#include <stdio.h>
#include "CuTest.h"

CuSuite* CuGetSuite();
CuSuite* TestUtilTestSuite();
CuSuite* VecTestSuite();
CuSuite* LexerUnitTestSuite();
CuSuite* LexerHelpersUnitTestSuite();
CuSuite* PostFixUnitTestSuite();

void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, CuGetSuite());
	CuSuiteAddSuite(suite, TestUtilTestSuite());
	CuSuiteAddSuite(suite, VecTestSuite());
	CuSuiteAddSuite(suite, LexerUnitTestSuite());
	CuSuiteAddSuite(suite, LexerHelpersUnitTestSuite());
	CuSuiteAddSuite(suite, PostFixUnitTestSuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}

int main(void)
{
	RunAllTests();
}
