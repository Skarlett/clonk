#include "clonk.h"
#include "onkstd/merge_sort.h"
#include "libtest/CuTest.h"

inline static void __test__mergesort(CuTest *tc, struct onk_test_buffers *_)
{
    uint16_t in[] = { 10, 32, 3, 5, 1 };
    uint16_t out[] = { 1, 3, 5, 10, 32 };
    onk_merge_sort_u16(in, 0, 5);

    CuAssert(
        tc, "merge sort failed",
        memcmp(in, out, sizeof(uint16_t) * 5) == 0
    );
}
