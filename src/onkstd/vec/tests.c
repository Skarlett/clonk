#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "libtest/CuTest.h"
#include "onkstd/vec.h"

void __test__vec_init(CuTest* tc)
{
    struct onk_vec_t vec;
    CuAssertTrue(tc, onk_vec_init(&vec, 4, 10) == 0);

    CuAssertTrue(tc, vec.base != 0);
    CuAssertTrue(tc, vec.capacity == 4);
    CuAssertTrue(tc, vec.type_sz == 10);
    /* CuAssertTrue(tc, vec.head == vec.base); */
    free(vec.base);
}

void __test__vec_push(CuTest* tc)
{
    struct onk_vec_t vec;
    uint8_t num = 1;
    char msg[64];

    onk_vec_init(&vec, 4, sizeof(uint8_t));
    // push 5 elements into a vec sized 4
    for(int i=0; 5 > i; i++) {
      memset(msg, 0, sizeof(char[64]));
      sprintf(msg, "failed on vec push, index [%d]", i);
      CuAssert(tc, msg, onk_vec_push(&vec, &num) != 0);
    }
    CuAssertTrue(tc, vec.capacity == 8);
    free(vec.base);
}

CuSuite* OnkVecTests(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__vec_init);
    SUITE_ADD_TEST(suite, __test__vec_push);
    return suite;
}
