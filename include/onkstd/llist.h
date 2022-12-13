#ifndef LLIST_H_
#define LLIST_H_

#include <stdint.h>
#include <stdbool.h>

struct onk_llist_t
{
    struct onk_llist_t *next;
    uint16_t i;
};

struct onk_llist_rm_t {
    struct onk_llist_t *root;
    bool removed;
};

struct onk_llist_rm_t onk_llist_rm(
    struct onk_llist_t *llist,
    uint16_t i
);

void onk_llist_init(
    struct onk_llist_t *bufarray,
    uint16_t n
);


uint16_t onk_llist_flatten(uint16_t *arr, struct onk_llist_t *llist, uint16_t arr_sz);
int16_t onk_llist_contains(struct onk_llist_t *llist, uint16_t i);

#endif // LLIST_H_
