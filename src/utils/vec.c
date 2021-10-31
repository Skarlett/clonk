#include "../prelude.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "vec.h"

int8_t init_vec(struct Vec *vec, usize capacity, uint16_t type_sz) {
    vec->type_sz = type_sz;
    vec->base = calloc(capacity, type_sz);
    vec->head = vec->base;
    vec->capacity = capacity;
    vec->len = 0;
    
    if (vec->base == 0)
        return -1;
    return 0;
}

int8_t vec_realloc(struct Vec *vec)
{
    void * ret = realloc(vec->base, (2 * vec->capacity) * vec->type_sz);
    
    if (ret == 0)
        return -1;
    
    vec->capacity = vec->capacity * 2;
    vec->base = ret;
    return 0;
}

void * vec_push(struct Vec *vec, void *src)
{
    void * ret;
    if ((vec->head - vec->base) + vec->type_sz > (vec->type_sz * vec->capacity) && vec_realloc(vec) == -1)
        return 0;
    
    if (memcpy(vec->head, src, vec->type_sz) == 0)
        return 0;
    ret = vec->head;

    vec->len += 1;
    vec->head += vec->type_sz;

    return ret;
}

void * vec_index(struct Vec *vec, usize idx)
{
    void * ret;
    
    // realloc
    if (idx > vec->capacity)
        return 0;
    
    return vec->base + (vec->type_sz * idx);
}