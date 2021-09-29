#include <stdio.h>

#include "CuTest.h"


CuSuite* LexerUnitTestSuite();
CuSuite* LexerHelpersUnitTestSuite();
CuSuite* FnMaskUnitTestSuite();
CuSuite* PostFixUnitTestSuite();

void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, LexerUnitTestSuite());
	CuSuiteAddSuite(suite, LexerHelpersUnitTestSuite());
	CuSuiteAddSuite(suite, FnMaskUnitTestSuite());
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
