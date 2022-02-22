#include <stdint.h>
#include <stdbool.h>

#define ONK_LAZY_REF_LIMIT 8
#define ONK_LAZY_SZ_LIMIT 512

enum onk_collection_no {
  onkstd_collection_udef_t,
  onkstd_collection_lazy_t,
  onkstd_collection_norm_t
};


struct _onk_collection_norm_t {
    const void * base;
};


/* lazily collect non-mutable arrays,
   and treat the totality of the collection
   as one sequential array.

   Reallocate into heap
   if size limit is exceeded.
*/
struct _onk_collection_lazy_t {
    /* {void *, } */
    const void * base[ONK_LAZY_REF_LIMIT];
    uint16_t sizes[ONK_LAZY_REF_LIMIT];
};


struct onkstd_collection_t {
    enum onk_collection_no type;
    union {
        struct _onk_collection_lazy_t lazy;
        struct _onk_collection_norm_t norm;
    } alloc;

    uint16_t item_sz;
    uint16_t nitems;
};


struct onk_iter_collection {
    /* {void *, } */
    const struct onkstd_collection_t *self;
    uint16_t i;
};


struct onk_iter_collection iter_lazy_collection
(const struct onkstd_collection_t *ptr)
{
    struct onk_iter_collection iter;
    iter.self = ptr;
    iter.i = 0;
    return iter;
}


bool onk_step_iter(struct onk_iter_collection *iter)
{

}



// for (int i=0; onk_step_iter(&iter, i); i++)
// {
//     item = onk_access_collection(&iter, i);
// }


int8_t step_lazy_collection(const struct )
