#include "clonk.h"
#include "onkstd/merge_sort.h"
#include "libtest/CuTest.h"
#include <stdint.h>

typedef void (*FnSort)(uint16_t *arr, uint16_t n);


void _test_sort(CuTest *tc, FnSort sort_fn, uint16_t line, const char * fp)
{
    uint16_t in[] = { 10, 32, 3, 5, 1 };
    uint16_t out[] = { 1, 3, 5, 10, 32 };

    sort_fn(in, 5);
    CuAssert(
        tc, "merge sort failed",
        memcmp(in, out, sizeof(uint16_t) * 5) == 0
    );
}

void __test__bubble_sort(CuTest *tc)
{
    _test_sort(tc, onk_bubble_sort_u16, __LINE__, __FILE__);
}

/* void __test__mergesort(CuTest *tc) */
/* { */
/*     uint16_t in[] = { 10, 32, 3, 5, 1 }; */
/*     uint16_t out[] = { 1, 3, 5, 10, 32 }; */

/*     onk_merge_sort_u16(in, 0, 5); */

/*     CuAssert( */
/*         tc, "merge sort failed", */
/*         memcmp(in, out, sizeof(uint16_t) * 5) == 0 */
/*     ); */
/* } */


CuSuite* OnkMergeSortTests(void) {
    CuSuite* suite = CuSuiteNew();
    //SUITE_ADD_TEST(suite, __test__mergesort);

    SUITE_ADD_TEST(suite, __test__bubble_sort);
    return suite;
}
