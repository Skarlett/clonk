#include "lexer.h"
#include "libtest/CuTest.h"
#include "onkstd/llist.h"
#include <assert.h>
#include <stdint.h>

void __test__llist_rm(CuTest *tc)
{
    struct onk_llist_t llbuf[5];
    struct onk_llist_t *root = 0;
    uint16_t buf[5] = {0, 1, 2, 3, 4}, buf2[5] = {0}, answer[5] = {1, 2, 4, 0, 0};

    struct onk_llist_rm_t rmret;

    uint8_t ctr = 0;

    onk_llist_init(llbuf, 5);
    onk_llist_rm(llbuf, 3);
    root = &llbuf[0];

    CuAssertTrue(tc, onk_llist_contains(root, 3) == -1);
    CuAssertTrue(tc, onk_llist_contains(root, 0) == 0);
    CuAssertTrue(tc, onk_llist_contains(root, 4) == 3);

    rmret = onk_llist_rm(root, 0);
    CuAssertPtrEquals(tc, &llbuf[1], rmret.root );
    CuAssertIntEquals(tc, 3, onk_llist_flatten(buf2, rmret.root, 5));
    CuAssertTrue(tc, memcmp(buf2, answer, 3) == 0);
}

CuSuite* OnkLListTests(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__llist_rm);
    return suite;
}
