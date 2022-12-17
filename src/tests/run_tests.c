#include "clonk.h"
#include "onkstd/vec.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include "libtest/CuTest.h"

/********************************/
/*  Test the function harness   */
/********************************/
CuSuite* CuGetSuite();
/******************/
/*  Test libtest  */
/******************/
CuSuite* OnkMaskAssertSuite();
CuSuite* OnkAssertTests();

/****************/
/*  Test utils  */
/****************/
CuSuite* OnkVecTests();
CuSuite* OnkMergeSortTests();
CuSuite* OnkLListTests();
/*****************/
/*   Test lexer  */
/*****************/
CuSuite* LexerUnitTests();
CuSuite* LexerHarnessLogicTests();


void RunAllTests(void)
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, CuGetSuite());
    CuSuiteAddSuite(suite, OnkMergeSortTests());
    CuSuiteAddSuite(suite, OnkLListTests());
    CuSuiteAddSuite(suite, OnkVecTests());
    CuSuiteAddSuite(suite, OnkAssertTests());
    
    // FIXME BROKEN
    // CuSuiteAddSuite(suite, OnkMaskAssertSuite());
    // CuSuiteAddSuite(suite, OnkAssertTests());
    CuSuiteAddSuite(suite, LexerUnitTests());
    CuSuiteAddSuite(suite, LexerHarnessLogicTests());

    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);

    printf("%s\n", output->buffer);
}

int main(void)
{
    RunAllTests();
    return 0;
}
