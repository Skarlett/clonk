#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "utils/vec.h"

#define VEC_STATE_FREED 1
#define VEC_STATE_INITALIZED 2
#define VEC_STATE_LOCK_ALLOC 3

int8_t onk_vec_init(struct onk_vec_t *vec, size_t capacity, size_t type_sz)
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

int8_t onk_vec_realloc(struct onk_vec_t *vec)
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

int8_t onk_vec_free(struct onk_vec_t *vec) {
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
void * onk_vec_push(struct onk_vec_t *vec, const void *src)
{
    void * ret;
    if (vec->state != VEC_STATE_INITALIZED || vec->base == 0 || vec->head == 0)
        return 0;

    else if ((vec->head - vec->base) + vec->type_sz > (vec->type_sz * vec->capacity) && onk_vec_realloc(vec) == -1)
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
** returns -1 if vector not initalized
 */
int8_t onk_vec_clear(struct onk_vec_t *vec) {
    if (vec->state == VEC_STATE_INITALIZED)
        vec->len = 0;
    else
        return -1;
    return 0;
}
