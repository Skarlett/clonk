#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <malloc.h>

#define ONK_LAZY_REF_LIMIT 8
#define ONK_LAZY_SZ_LIMIT 512

enum onk_collection_no {
  onkstd_collection_udef_t,
  onkstd_collection_shallow_t,
  onkstd_collection_norm_t
};


struct _onk_collection_norm_t {
    const void * base;
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
        struct _onk_collection_norm_t norm;
    } alloc;

    uint16_t item_sz;
    uint16_t nitems;
};


struct onk_iter_collection_t {
    /* {void *, } */
    const struct onkstd_collection_t *self;
    uint16_t i;
};


struct onk_iter_collection_t onk_iter_collection
(const struct onkstd_collection_t *ptr)
{
    struct onk_iter_collection_t iter;
    iter.self = ptr;
    iter.i = 0;
    return iter;
}

/*
  for the collection to be normalized
  into a deep-copy.
*/
int8_t onk_collection_deep_copy(struct onkstd_collection_t *col)
{
    void * heap;
    const void *slice;
    uint16_t nslices = 0, size, ncompleted = 0;

    if(col->type != onkstd_collection_shallow_t)
        return 0;

    heap = calloc(col->nitems+1, col->item_sz);
    nslices = col->alloc.lazy.slices;

    for (uint16_t i=0; nslices > i; i++)
    {
        size = col->alloc.lazy.sizes[i];
        slice = col->alloc.lazy.base[i];

        memcpy(
            heap + (ncompleted * col->item_sz),
            slice,
            size * col->item_sz
        );

        ncompleted += size;
    }

}

bool onk_step_iter(
    struct onk_iter_collection *iter,
    uint16_t i
)
{


}


void * onk_access_collection()
{}


// for (int i=0; onk_step_iter(&iter, i); i++)
// {
//     item = onk_access_collection(&iter, i);
// }


int8_t step_lazy_collection(const struct )
