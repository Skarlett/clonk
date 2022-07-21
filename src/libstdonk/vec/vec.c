#include "clonk.h"
#include "onkstd/vec.h"
#include "onkstd/int.h"

#define VEC_INC_NORM 256
#define VEC_INC_HUGE 8192
#define VEC_MAX 1 << 14

int8_t can_access(struct onk_vec_t *vec)
{ return vec->state == ONK_VEC_READY; }

int8_t onk_vec_init(
    struct onk_vec_t *vec,
    uint16_t capacity,
    uint16_t type_sz
){
    vec->type_sz = type_sz;
    vec->base = calloc(capacity, type_sz);
    vec->capacity = capacity;
    vec->len = 0;
    vec->inc = 0;
    vec->clamp = VEC_MAX;
    vec->state = ONK_VEC_READY;
    
    if (vec->base == 0)
        return -1;

    return 0;
}

/*
**
** returns -3 if marked as unallocated/freed
**         -2 if `inc` added to `vec->capacity` overflows unsigned 16bit
**         -1 if `new_capacity >= vec->clamp_at`
**          0 if realloc returned null-ptr or `errno==ENOMEM`
**          1 if successful
**
*/
int8_t onk_vec_realloc(struct onk_vec_t *vec, uint16_t inc)
{
    void *ret = 0;
    bool overflowed = false;
    uint16_t new_capacity = \
        onk_add_u16(vec->capacity, inc, &overflowed);

    /* double free? */
    if(!can_access(vec))
        return -3;

    /* will overflow */
    else if(overflowed)
        return -2;

    /* limited by clamp */
    else if(vec->clamp > 0 && new_capacity >= vec->clamp)
        return -1;

    ret = realloc(
        vec->base,
        new_capacity * vec->type_sz
    );

    if(errno == ENOMEM || ret == 0)
        return 0;

    /* moved out of place */
    if(ret != vec->base)
    {
        memcpy(ret, vec->base, vec->capacity * vec->type_sz);
        vec->base = ret;
    }

    vec->capacity = new_capacity;
    return 1;
}


int8_t _resize(struct onk_vec_t *dest)
{
    int8_t ret;
    /* reallocate */
    if(dest->inc > 0)
        ret = onk_vec_realloc(dest, dest->inc);

    else if(dest->capacity > VEC_INC_HUGE)
        ret = onk_vec_realloc(dest, VEC_INC_HUGE);

    else
        ret = onk_vec_realloc(dest, VEC_INC_NORM);

    return ret;
}

void * _push(struct onk_vec_t *dest, const void *src)
{

    void * item = (onk_usize)dest->base + (onk_usize)(dest->len * dest->type_sz);

    assert(memcpy(
            item,
            src,
            dest->type_sz) > 0
    );

    dest->len += 1;
    return item;
}

/*
** Push item into expandable buffer
** returns pointer to item in expandable buffer
** otherwise returns 0 to indicate error.
*/
void * onk_vec_push(struct onk_vec_t *dest, const void *src)
{
    if(can_access(dest) == false)
        return 0;

    /* write into buffer */
    if(dest->capacity > dest->len)
        return _push(dest, src);

    /* realloc */
    else if (_resize(dest) != 1)
        return 0;

    /* and then write */
    return _push(dest, src);
}


/*

*/
void * onk_vec_copy(struct onk_vec_t *dest, const struct onk_vec_t *src)
{
    if(can_access(dest) == true || can_access(src) == false)
        return 0;

    return memcpy(dest, src, sizeof(struct onk_vec_t));
}

/*

*/
void * onk_vec_deep_copy(struct onk_vec_t *dest, const struct onk_vec_t *src)
{

    if (onk_vec_copy(dest, src) == 0)
        return 0;

    dest->base = malloc(src->type_sz * src->capacity);
    
    if(dest->base == 0) 
      return 0;

    return memcpy(dest->base, src->base, src->type_sz * src->len);
}


/*
** Push item into expandable buffer
** returns pointer to item in expandable buffer
** otherwise returns 0 to indicate error.
*/

/* int8_t onk_vec_extend(struct onk_vec_t *dest, struct onk_vec_t *src) */
/* { */
/*     onk_unimplemented(); */
/*     /\* if (vec->state != VEC_STATE_INITALIZED || vec->base == 0) *\/ */
/*     /\*     return -1; *\/ */

/*     /\* void * ret = realloc(vec->base, sizeof(vec->capacity * vec->type_sz * 2)); *\/ */

/*     /\* if (ret == 0) *\/ */
/*     /\*     return -1; *\/ */

/*     /\* vec->capacity = vec->capacity * 2; *\/ */
/*     /\* vec->base = ret; *\/ */
/*     /\* return 0; *\/ */
/* } */

/*
** return last valid item in buffer
** returns 0 if empty
*/
const void * onk_vec_head(const struct onk_vec_t *vec)
{
    if (vec->len > 0)
        return vec->base + vec->type_sz * vec->len - 1;
    return 0;
}


/*
** copy last inserted item from expandable buffer
** into `dest`, and remove the item from buffer.
** return -1 if empty.
**
** if `dest` is equal to 0,
** no copy is performed.
**
*/
int8_t onk_vec_pop(struct onk_vec_t *vec, void * dest) {
    const void *head = onk_vec_head(vec);

    if (head == 0 || dest == 0)
        return -1;

    else if (memcpy(dest, head, vec->type_sz) == 0)
        return -1;

    vec->len -= 1;
    return 0;
}

/*
** clears expandable buffer.
 */
void onk_vec_clear(struct onk_vec_t *vec)
{ vec->len = 0; }

void onk_vec_clamp(struct onk_vec_t *vec, uint16_t max)
{ vec->clamp = max; }

/*
** clears expandable buffer.
**
** returns -1 if vector not initalized
*/
void onk_vec_reset(struct onk_vec_t *vec) {
    vec->len = 0;
    vec->type_sz = 0;
    vec->clamp = 0;
}

int8_t onk_vec_free(struct onk_vec_t *vec) {
    if(!can_access(vec))
        return -1;

    free(vec->base);
    onk_vec_reset(vec);
    vec->state = ONK_VEC_UNINIT;
    vec->base = 0;
    vec->capacity = 0;
    return 0;
}
