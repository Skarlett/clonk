// #include "linked_list.h"
#include <stdint.h>
#include <stdbool.h>
#include "onkstd/llist.h"

void onk_llist_init(
    struct onk_llist_t *bufarray,
    uint16_t n
)
{
    for (uint16_t i=0; n > i; i++)
    {
        bufarray[i].next = 0;
        bufarray[i].i = i;

        if(n-1 > i)
            bufarray[i].next = &bufarray[i+1];
    }
}

int16_t onk_llist_contains(struct onk_llist_t *llist, uint16_t i)
{
    struct onk_llist_t *step = llist;
    int16_t ctr=0;

    while(step)
    {
        if(step->i == i)
            return ctr;

        if(step->next)
          step = step->next;
        else break;

        ctr += 1;
    }
    return -1;
}

uint16_t onk_llist_flatten(uint16_t *arr, struct onk_llist_t *llist, uint16_t arr_sz)
{
    struct onk_llist_t *step=llist;
    uint16_t ctr = 0;
    while(step) {
        arr[ctr++] = step->i;
        if(step->next)
          step = step->next;
        else break;
    }
    return ctr;
}

struct onk_llist_rm_t onk_llist_rm(
    struct onk_llist_t *llist,
    uint16_t i
)
{
    struct onk_llist_t *step=llist, *prev = 0;
    struct onk_llist_rm_t ret;

    ret.root = llist;
    ret.removed = false;

    while (step)
    {
        if (i == step->i)
        {
            if (prev)
                prev->next = step->next;
            else
                ret.root = step->next;

            ret.removed = true;
            return ret;
        }

        if(step->next)
        {
            prev = step;
            step = step->next;
        }
        else break;
    }

    return ret;
}
