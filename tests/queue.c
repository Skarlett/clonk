#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "testutils.h"
#include "CuTest.h"
#include "../src/prelude.h"
#include "../src/utils/queue.h"


void __test__queue_push(CuTest* tc) {
    struct Vec vec;
    uint8_t num = 1;
    char msg[64];

    // push 5 elements into a vec sized 4
    for(int i=0; 5 > i; i++) {
      memset(msg, 0, sizeof(char[64]));
      sprintf(msg, "failed on vec push, index [%d]", i);
      CuAssert(tc, msg, vec_push(&vec, &num) != 0);
    }
    CuAssertTrue(tc, vec.capacity == 8);
    free(vec.base);
}



CuSuite* VecTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__queue_push);
    return suite;
}
