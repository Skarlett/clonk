#include "clonk.h"
#include "libtest/CuTest.h"
#include "onkstd/vec.h"
#include "parser.h"
#include <assert.h>
#include <errno.h>

/********************************/
/*  Test the function harness   */
/********************************/
CuSuite* CuGetSuite();
CuSuite* CuGetCtxSuite();

/******************/
/*  Test libtest  */
/******************/
CuSuite* OnkMaskAssertSuite();
CuSuite* OnkAssertTests();

/****************/
/*  Test utils  */
/****************/
CuSuite* OnkVecTests();
CuSuite* OnkQueueTest();
CuSuite* OnkMergeSortTests();

/*****************/
/*   Test lexer  */
/*****************/
CuSuite* LexerUnitTests();
CuSuite* LexerHarnessUBTests();
CuSuite* LexerHarnessLogicTests();

void RunAllTests(void)
{
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();

    CuSuiteAddSuite(suite, CuGetSuite());
    // FIXME BROKEN
    // CuSuiteAddSuite(suite, OnkMaskAssertSuite());
    // CuSuiteAddSuite(suite, OnkAssertTests());

    CuSuiteAddSuite(suite, OnkVecTests());
    CuSuiteAddSuite(suite, OnkQueueTest());
    CuSuiteAddSuite(suite, OnkMergeSortTests());
    CuSuiteAddSuite(suite, LexerUnitTests());

    CuSuiteAddSuite(suite, LexerHarnessUBTests());
    CuSuiteAddSuite(suite, LexerHarnessLogicTests());
    /* CuSuiteAddSuite(suite, PostfixTests()); */

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
