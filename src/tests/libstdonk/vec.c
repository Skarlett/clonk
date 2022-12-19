#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "libtest/CuTest.h"
#include "onkstd/vec.h"

void __test__vec_init(CuTest* tc)
{
    struct onk_vec_t vec;
    onk_vec_init(&vec);
    CuAssertPtrEquals(tc, 0, vec.base);
    CuAssertIntEquals(tc, 0, vec.capacity);
    CuAssertIntEquals(tc, 0, vec.type_sz);
    CuAssertIntEquals(tc, onk_vec_mode_uninit, vec.state);
}

void __test__vec_free_heap(CuTest* tc)
{
    struct onk_vec_t vec;

    onk_vec_init(&vec);
    onk_vec_new(&vec, 4, sizeof(uint8_t));
    CuAssertPtrNotNull(tc, vec.base);
    CuAssertIntEquals(tc, 4, vec.capacity);
    CuAssertIntEquals(tc, sizeof(uint8_t), vec.type_sz);
    CuAssertTrue(tc, vec.state == onk_vec_mode_alloc_heap);

    onk_vec_free(&vec);
    CuAssertPtrEquals(tc, 0, vec.base);
    CuAssertIntEquals(tc, 0, vec.capacity);
    CuAssertIntEquals(tc, 0, vec.type_sz);
    CuAssertTrue(tc, vec.state == onk_vec_mode_uninit);
}

void __test__vec_free_stack(CuTest* tc)
{
    struct onk_vec_t vec;
    uint8_t buf[4];

    onk_vec_init(&vec);
    onk_vec_new_stk(&vec, buf, 4, sizeof(uint8_t));
    CuAssertPtrNotNull(tc, vec.base);
    CuAssertIntEquals(tc, 4, vec.capacity);
    CuAssertIntEquals(tc, sizeof(uint8_t), vec.type_sz);
    CuAssertTrue(tc, vec.state == onk_vec_mode_alloc_stack);

    onk_vec_free(&vec);
    CuAssertPtrEquals(tc, 0, vec.base);
    CuAssertIntEquals(tc, 0, vec.capacity);
    CuAssertIntEquals(tc, 0, vec.type_sz);
    CuAssertTrue(tc, vec.state == onk_vec_mode_uninit);
}

void __test__vec_pushpop_stack(CuTest* tc)
{
    struct onk_vec_t vec;
    uint8_t stack[4];
    uint8_t *push_ptr = 0, *pop_ptr = 0;
    uint8_t dest = 0, src=12;

    onk_vec_new_stk(&vec, stack, 4, sizeof(uint8_t));
    
    push_ptr = (void*)onk_vec_push(&vec, &src);
    CuAssertPtrEquals(tc, vec.base, push_ptr);
    CuAssertIntEquals(tc, src, *push_ptr);
    
    CuAssertIntEquals(tc, 1, vec.len);
    CuAssertIntEquals(tc, 4, vec.capacity);

    onk_vec_pop(&vec, &dest);
    CuAssertIntEquals(tc, 0, vec.len);
    CuAssertIntEquals(tc, 4, vec.capacity);
    CuAssertIntEquals(tc, src, dest);
    onk_vec_free(&vec);
}

void __test__vec_pushpop_heap(CuTest* tc)
{
    struct onk_vec_t vec;
    void *ret_ptr = 0;
    uint8_t dest = 0, src=12;

    onk_vec_new(&vec, 4, sizeof(uint8_t));

    ret_ptr = (void*)onk_vec_push(&vec, &src);
    CuAssertTrue(tc, ret_ptr != 0);
    CuAssertTrue(tc, vec.len == 1);

    onk_vec_pop(&vec, &dest);
    CuAssertTrue(tc, vec.len == 0);
    CuAssertTrue(tc, vec.capacity == 4);
    CuAssertTrue(tc, dest == src);

    onk_vec_free(&vec);
}

void __test__vec_realloc_stack(CuTest* tc)
{
    struct onk_vec_t vec;
    uint8_t num = 1, stack[4];

    onk_vec_new_stk(&vec, stack, 4, sizeof(uint8_t));
    CuAssertTrue(tc, vec.state == onk_vec_mode_alloc_stack);
    vec.len = 4;
    vec.inc = 0;

    onk_vec_push(&vec, &num);
    
    CuAssertTrue(tc, vec.state == onk_vec_mode_alloc_heap);
    CuAssertTrue(tc, vec.capacity == 8);
    onk_vec_free(&vec);
}

void __test__vec_realloc_heap(CuTest* tc)
{
    struct onk_vec_t vec;
    uint8_t num = 1;

    onk_vec_new(&vec, 4, sizeof(uint8_t));
    CuAssertTrue(tc, vec.state == onk_vec_mode_alloc_heap);
    vec.len = 4;
    vec.inc = 0;

    onk_vec_push(&vec, &num);

    CuAssertTrue(tc, vec.state == onk_vec_mode_alloc_heap);
    CuAssertTrue(tc, vec.capacity == 8);
    onk_vec_free(&vec);
}

CuSuite* OnkVecTests(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__vec_init);
    
    SUITE_ADD_TEST(suite, __test__vec_free_heap);
    SUITE_ADD_TEST(suite, __test__vec_free_stack);
    
    SUITE_ADD_TEST(suite, __test__vec_pushpop_stack);
    SUITE_ADD_TEST(suite, __test__vec_pushpop_heap);

    SUITE_ADD_TEST(suite, __test__vec_realloc_stack);
    SUITE_ADD_TEST(suite, __test__vec_realloc_heap);
    
    SUITE_ADD_TEST(suite, __test__vec_realloc_stack);
    SUITE_ADD_TEST(suite, __test__vec_realloc_heap);
    return suite;
}
