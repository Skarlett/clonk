#include "clonk.h"
#include "onkstd/vec.h"
#include "onkstd/int.h"
#include <stdint.h>

#define VEC_INC_SMALL 256
#define VEC_MAX_CAPACITY 1 << 14

int can_access(const struct onk_vec_t *vec)
{ return vec->state == onk_vec_mode_alloc_stack || vec->state == onk_vec_mode_alloc_heap; }


void onk_vec_init(struct onk_vec_t *vec)
{
    vec->type_sz = 0;
    vec->base = 0;
    vec->capacity = 0;
    vec->len = 0;
    vec->clamp = VEC_MAX_CAPACITY;
    vec->state = onk_vec_mode_uninit;
    vec->inc = 0;
}

void onk_vec_small(struct onk_vec_t *vec)
{ vec->inc = VEC_INC_SMALL; }

void _onk_vec_alloc(
    struct onk_vec_t *vec,
    enum onk_vec_mode_t flag,
    uint16_t capacity,
    uint16_t type_sz
){
    vec->type_sz = type_sz;
    vec->capacity = capacity;
    vec->state = flag;
}

void onk_vec_alloc_heap(
    struct onk_vec_t *vec,
    uint16_t capacity,
    uint16_t type_sz
){
    _onk_vec_alloc(vec, onk_vec_mode_alloc_heap, capacity, type_sz);
    vec->base = calloc(capacity, type_sz);
}

void onk_vec_alloc_stk(struct onk_vec_t *vec, void *stack_ptr, uint16_t capacity, uint16_t type_sz)
{
    _onk_vec_alloc(vec, onk_vec_mode_alloc_stack, capacity, type_sz);
    vec->base = stack_ptr;
}

void onk_vec_new(
    struct onk_vec_t *vec,
    uint16_t capacity,
    uint16_t type_sz
){
    onk_vec_init(vec);
    onk_vec_alloc_heap(vec, capacity, type_sz);
    assert(vec->base);
}

void onk_vec_new_stk(
    struct onk_vec_t *vec,
    void *stack_ptr,
    uint16_t capacity,
    uint16_t type_sz
){
    onk_vec_init(vec);
    onk_vec_alloc_stk(vec, stack_ptr, capacity, type_sz);
}

uint16_t _calc_realloc_slots(struct onk_vec_t *vec)
{
    bool overflowed = false;
    uint16_t new_capacity = 0;   

    if (vec->inc == 0)
      new_capacity = onk_add_u16(vec->capacity, vec->capacity, &overflowed);
    else
      new_capacity = onk_add_u16(vec->capacity, vec->inc, &overflowed);

    /* will overflow */
    if(overflowed)
        return 0;

    if (vec->clamp && new_capacity > vec->clamp) 
        return vec->clamp;
    
    return new_capacity;   
}

/*
** returns -3 if marked as unallocated/freed
**         -2 if `inc` added to `vec->capacity` overflows unsigned 16bit
**         -1 if `new_capacity >= vec->clamp_at`
**          0 if realloc returned null-ptr or `errno==ENOMEM`
**          1 if successful
*/
int8_t _realloc(struct onk_vec_t *vec)
{
    void *ret = 0;
    bool overflowed = false;
    uint16_t new_capacity = 0;   

    new_capacity = _calc_realloc_slots(vec);
    
    if(new_capacity == 0)
      return -2;
    
    if(vec->state == onk_vec_mode_alloc_stack)
    {   
        struct onk_vec_t stack_copy;
        onk_vec_init(&stack_copy);
        onk_vec_copy(&stack_copy, vec);
        onk_vec_reset(vec);

        stack_copy.capacity = new_capacity;
        onk_vec_deep_copy(vec, &stack_copy);
        return 1;
    }

    assert(vec->state == onk_vec_mode_alloc_heap);
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

void * _push(struct onk_vec_t *dest, const void *src)
{
    void * item = dest->base + dest->len * dest->type_sz;
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
    assert(can_access(dest));
    assert(src);

    /* write into buffer */
    if(dest->capacity > dest->len)
        return _push(dest, src);

    /* realloc */
    else if (_realloc(dest) != 1)
        return 0;

    /* and then write */
    return _push(dest, src);
}

void * onk_vec_copy(
    struct onk_vec_t *dest,
    const struct onk_vec_t *src)
{
    assert(dest->state == onk_vec_mode_uninit);
    assert(can_access(src));
    return memcpy(dest, src, sizeof(struct onk_vec_t));
}

void * onk_vec_deep_copy(struct onk_vec_t *dest, const struct onk_vec_t *src)
{
    assert(dest->state == onk_vec_mode_uninit);
    assert(can_access(src));

    if (onk_vec_copy(dest, src) == 0)
        return 0;

    dest->base = calloc(src->capacity, src->type_sz);
    dest->state = onk_vec_mode_alloc_heap;
    if(dest->base == 0) 
      return 0;

    return memcpy(dest->base, src->base, src->type_sz * src->len);
}

void onk_vec_deep_copy_resize(
    struct onk_vec_t *dest,
    struct onk_vec_t *src
){    
    uint16_t old_capacity = 0;
    uint16_t new_capacity = 0;

    old_capacity = src->capacity;
    new_capacity = _calc_realloc_slots(src);
    
    assert(old_capacity);
    assert(new_capacity);

    src->capacity = new_capacity;
    onk_vec_deep_copy(dest, src);
    src->capacity = old_capacity;
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
    assert(can_access(vec));
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
int8_t onk_vec_pop(struct onk_vec_t *vec, void * dest)
{
    const void *head = onk_vec_head(vec);
    assert(head != 0 || dest != 0);
    assert(memcpy(dest, head, vec->type_sz));
    vec->len -= 1;
    return 0;
}

void onk_vec_move(struct onk_vec_t *dest, struct onk_vec_t *src)
{
    assert(src->state == onk_vec_mode_uninit);
    onk_vec_copy(dest, src);
    src->state = onk_vec_mode_free;
}

void onk_vec_clear(struct onk_vec_t *vec){
    vec->len = 0;
}

void onk_vec_clamp(struct onk_vec_t *vec, uint16_t max){
    vec->clamp = max;
}

void onk_vec_reset(struct onk_vec_t *vec){
    //assert(vec->state == onk_vec_mode_free);
    
    vec->state = onk_vec_mode_uninit;
    vec->len = 0;
    vec->type_sz = 0;
}

void onk_vec_free(struct onk_vec_t *vec)
{
    
    if (vec->state == onk_vec_mode_alloc_stack)
      onk_nop;
    else {
        assert(can_access(vec));
        free(vec->base);
    }
    
    vec->state = onk_vec_mode_free;
    onk_vec_reset(vec);
    
    vec->base = 0;
    vec->capacity = 0;
}
