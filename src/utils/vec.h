#ifndef _HEADER__VEC__
#define _HEADER__VEC__
#include <stdint.h>
#include "../prelude.h"

struct Vec {
    void *base;
    void *head;
    uint16_t type_sz;
    usize capacity;
    usize len;
};

int8_t init_vec(struct Vec *vec, usize capacity, uint16_t type_sz);
int8_t vec_realloc(struct Vec *vec);
void * vec_push(struct Vec *vec, void *src);
void * vec_index(struct Vec *vec, usize idx);

#endif