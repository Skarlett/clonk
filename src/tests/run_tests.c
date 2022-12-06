#include "clonk.h"
#include "onkstd/vec.h"
#include "parser.h"
#include <errno.h>
#include <stdio.h>
#include "libtest/CuTest.h"

/* CuTest* _last_tc = 0; */
/* CuTest * _get_last_tc() */
/* { */
/*      void *tmp = _last_tc; */
/*      if (tmp == 0) */
/*      { */
/*         exit(-1); */
/*      } */
/*      _last_tc = 0; */
/*     return tmp; */
/* } */

/* #include <libunwind.h> */
/* void show_backtrace(void) { */
/*   unw_cursor_t cursor; */
/*   unw_context_t uc; */
/*   unw_word_t ip, sp; */

/*   unw_getcontext(&uc); */
/*   unw_init_local(&cursor, &uc); */

/*   while (unw_step(&cursor) > 0) { */
/*     unw_get_reg(&cursor, UNW_REG_IP, &ip); */
/*     unw_get_reg(&cursor, UNW_REG_SP, &sp); */
/*     printf("ip = %lx, sp = %lx\n", (long) ip, (long) sp); */
/*   } */
/* } */

/* void _assert_handler(CuTest *tc, int cond, char * fp, int line) { */
/*     printf("hello world"); */
/* } */



/* void _assert_handler(CuTest *tc, int cond, char * fp, int line) { */
/*     char msg[2048] = {0}; */
/*     snprintf(msg, 2048, "HELLO?!?!\nFAILED:%u:%s ", line, fp); */
/*     if (!cond) { */
/*         //show_backtrace(); */
/*         CuFail(tc, msg); */
/*     } */
/* } */

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

    assert(8*22);

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
