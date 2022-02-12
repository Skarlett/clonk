#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "vec.h"

#define VEC_STATE_FREED 1
#define VEC_STATE_INITALIZED 2
#define VEC_STATE_LOCK_ALLOC 3

int8_t init_vec(struct Vec *vec, size_t capacity, size_t type_sz)
{
    vec->type_sz = type_sz;
    vec->base = calloc(capacity, type_sz);
    vec->head = vec->base;
    vec->capacity = capacity;
    vec->len = 0;
    vec->state = VEC_STATE_INITALIZED;
    
    if (vec->base == 0)
        return -1;
    return 0;
}

int8_t vec_realloc(struct Vec *vec)
{
    if (vec->state != VEC_STATE_INITALIZED || vec->base == 0)
        return -1;
    
    void * ret = realloc(vec->base, sizeof(char[vec->capacity][vec->type_sz]) * 2);
    
    if (ret == 0)
        return -1;
    
    vec->capacity = vec->capacity * 2;
    vec->base = ret;
    return 0;
}

int8_t vec_free(struct Vec *vec) {
    if (vec->state != VEC_STATE_INITALIZED || vec->base == 0)
        return -1;
    
    free(vec->base);
    vec->state = VEC_STATE_FREED;
    return 0;
}

/*
** Push item into expandable buffer
** returns pointer to item in expandable buffer
** otherwise returns 0 to indicate error.
*/
void * vec_push(struct Vec *vec, const void *src)
{
    void * ret;
    if (vec->state != VEC_STATE_INITALIZED || vec->base == 0 || vec->head == 0)
        return 0;

    else if ((vec->head - vec->base) + vec->type_sz > (vec->type_sz * vec->capacity) && vec_realloc(vec) == -1)
        return 0;
    
    if (memcpy(vec->head, src, vec->type_sz) == 0)
        return 0;
    
    ret = vec->head;

    vec->len += 1;
    vec->head += vec->type_sz;

    return ret;
}

/*
** return last valid item in buffer
** returns 0 if empty
*/
const void * vec_head(const struct Vec *vec)
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
** NOTE: does not zero out data
*/
int8_t vec_pop(struct Vec *vec, void * dest) {
    const void *head = vec_head(vec);
    if (head != 0){
        if (dest != 0)
            assert(memcpy(dest, head, vec->type_sz) != 0);

        vec->len -= 1;
        return 0;
    }
    return -1;
}

/*
** clears expandable buffer.
**
** NOTE: does not zero out data.
** returns -1 if vector not initalized
 */
int8_t vec_clear(struct Vec *vec) {
    if (vec->state == VEC_STATE_INITALIZED)
        vec->len = 0;
    else
        return -1;
    return 0;
}
