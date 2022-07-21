#include <stdint.h>
#include "clonk.h"
#include <string.h>
#include <malloc.h>
#include "onkstd/int.h"

#define ONK_LAZY_REF_LIMIT 8
#define ONK_LAZY_SZ_LIMIT 512

enum onk_collection_no {
  onkstd_collection_udef_t,
  onkstd_collection_shallow_t,
  onkstd_collection_norm_t
};


/* lazily collect non-mutable arrays,
   and treat the totality of the collection
   as one sequential array.

   # Reallocate into heap
   # if size limit is exceeded.
*/
struct _onk_collection_shallow_t {
    /* {void *, } */
    const void ** base;

    /* list of lengths */
    uint16_t * sizes;

    /* how many items inside base*/
    uint16_t slices;
};

struct onkstd_collection_t {
    enum onk_collection_no type;
    union {
        struct _onk_collection_shallow_t lazy;
        const void * norm;
    } alloc;

    uint16_t item_sz;
    uint16_t total_items;
};


struct onk_collection_shallow_iter_t
{

};




/*
  for the collection to be normalized
  into a deep-copy.
*/
int8_t onk_collection_deep_copy(struct onkstd_collection_t *col, void * buffer)
{
    const void *slice;
    uint16_t nslices = 0, size, ncompleted = 0;

    if(col->type != onkstd_collection_shallow_t)
        return 0;

    //heap = calloc(col->nitems+1, col->item_sz);
    nslices = col->alloc.lazy.slices;

    /* copy each slice */
    for (uint16_t i=0; nslices > i; i++)
    {
        size = col->alloc.lazy.sizes[i];
        slice = col->alloc.lazy.base[i];

        memcpy(
            buffer + (ncompleted * col->item_sz),
            slice,
            size * col->item_sz
        );

        ncompleted += size;
    }

    col->type = onkstd_collection_norm_t;
    col->alloc.norm = buffer;

    return 0;
}


void * onk_access_collection(struct onkstd_collection_t *iter, uint16_t i)
{
    uint16_t accumulator = 0;
    struct _onk_collection_shallow_t *lazy;


    if(iter->type == onkstd_collection_norm_t)
    {
        return (void *)iter->alloc.norm + (iter->item_sz * i);
    }
    else if(iter->type == onkstd_collection_shallow_t)
    {

        lazy = &iter->alloc.lazy;

        for (uint16_t i=0; lazy->slices > i; i++)
        {
            accumulator = onkstd_add_u16(accumulator, lazy->sizes[i]);

        }

        //return iter->alloc.base[];
    }
}


// for (int i=0; iter->total_items > i ; i++)
// {
//     item = onk_access_collection(&iter, i);
// }


//int8_t step_lazy_collection(const struct )
