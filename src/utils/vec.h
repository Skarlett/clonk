#ifndef _HEADER__VEC__
#define _HEADER__VEC__
#include <stdint.h>
#include <sys/types.h>
#include "../prelude.h"

#define VEC_STATE_INITALIZED 1
#define VEC_STATE_FREED 2

struct Vec {
    void *base;
    void *head;
    uint16_t type_sz;
    usize capacity;
    usize len;

    uint8_t state;
};

int8_t init_vec(struct Vec *vec, usize capacity, uint16_t type_sz);
int8_t vec_realloc(struct Vec *vec);
void * vec_push(struct Vec *vec, void *src);
void * vec_index(struct Vec *vec, usize idx);

#endif