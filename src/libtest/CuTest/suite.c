#include "libtest/CuTest.h"

/*-------------------------------------------------------------------------*
 * CuSuite
 *-------------------------------------------------------------------------*/
void CuSuiteInit(CuSuite* testSuite)
{
    testSuite->assertCount = 0;
    testSuite->count = 0;
    testSuite->failCount = 0;
    memset(testSuite->list, 0, sizeof(testSuite->list));
}

CuSuite* CuSuiteNew(void)
{
    CuSuite* testSuite = CU_ALLOC(CuSuite);
    CuSuiteInit(testSuite);
    return testSuite;
}

void CuSuiteDelete(CuSuite *testSuite)
{
        unsigned int n = 0;
        for (n=0; n < MAX_TEST_CASES; n++)
        {
                if (testSuite->list[n])
                {
                        CuTestDelete(testSuite->list[n]);
                }
        }
        free(testSuite);
}

void CuSuiteAdd(CuSuite* testSuite, CuTest *testCase)
{
    assert(testSuite->count < MAX_TEST_CASES);
    testSuite->list[testSuite->count] = testCase;
    testSuite->count++;
}

void CuSuiteAddSuite(CuSuite* testSuite, CuSuite* testSuite2)
{
    int i = 0;
    for (i = 0 ; i < testSuite2->count ; ++i)
    {
        CuTest* testCase = testSuite2->list[i];
        CuSuiteAdd(testSuite, testCase);
    }
}

void CuSuiteRun(CuSuite* testSuite)
{
    int i = 0;
    testSuite->assertCount = 0;
    for (i = 0 ; i < testSuite->count ; ++i)
    {
        CuTest* testCase = testSuite->list[i];
        CuTestRun(testCase);
        testSuite->assertCount += testCase->assertCount;
        if (testCase->failed) { testSuite->failCount += 1; }
    }
}

void CuSuiteSummary(CuSuite* testSuite, CuString* summary)
{
    int i = 0;
    for (i = 0 ; i < testSuite->count ; ++i)
    {
        CuTest* testCase = testSuite->list[i];
        CuStringAppend(summary, testCase->failed ? "F" : ".");
    }
    CuStringAppend(summary, "\n\n");
}

void CuSuiteDetails(CuSuite* testSuite, CuString* details)
{
    int i = 0;
    int failCount = 0;

    if (testSuite->failCount == 0)
    {
        int passCount = testSuite->count - testSuite->failCount;
        const char* testWord = passCount == 1 ? "test" : "tests";
        CuStringAppendFormat(details, "OK (%d %s) (%u asserts)\n", passCount, testWord, testSuite->assertCount);
    }
    else
    {
        if (testSuite->failCount == 1)
            CuStringAppend(details, "There was 1 failure:\n");
        else
            CuStringAppendFormat(details, "There were %d failures:\n", testSuite->failCount);

        for (i = 0 ; i < testSuite->count ; ++i)
        {
            CuTest* testCase = testSuite->list[i];
            if (testCase->failed)
            {
                failCount++;
                CuStringAppendFormat(details, "%d) %s: %s\n",
                    failCount, testCase->name, testCase->message);
            }
        }
        CuStringAppend(details, "\n!!!FAILURES!!!\n");

        CuStringAppendFormat(details, "Runs: %d ",   testSuite->count);
        CuStringAppendFormat(details, "Fails: %d\n",  testSuite->failCount);
        CuStringAppendFormat(details, "Asserts: %u\n",  testSuite->assertCount);
    }
}
