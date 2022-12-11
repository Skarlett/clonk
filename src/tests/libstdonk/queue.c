#include "libtest/CuTest.h"
#include "onkstd/queue.h"

void __test__queue_push(CuTest* tc) {
    struct onk_vec_t vec;
    uint8_t num = 1;
    char msg[64];

    // push 5 elements into a vec sized 4
    for(uint8_t i=0; 5 > i; i++) {
      memset(msg, 0, sizeof(char[64]));
      sprintf(msg, "failed on vec push, index [%d]", i);
      CuAssert(tc, msg, onk_vec_push(&vec, &num) != 0);
    }

    CuAssertTrue(tc, vec.capacity == 8);
    free(vec.base);
}

CuSuite* OnkQueueTest(void) {
  CuSuite* suite = CuSuiteNew();
  // SUITE_ADD_TEST(suite, __test__queue_push);

  return suite;
}
