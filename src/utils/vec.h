#ifndef _HEADER__VEC__
#define _HEADER__VEC__
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include "../prelude.h"

struct Vec {
    void *base;
    void *head;
    uint16_t type_sz;
    size_t capacity;
    size_t len;

    uint8_t state;
};

int8_t init_vec(struct Vec *vec, size_t capacity, size_t type_sz);
int8_t vec_realloc(struct Vec *vec);
void * vec_push(struct Vec *vec, void *src);
void * vec_index(struct Vec *vec, usize idx);
int8_t vec_free(struct Vec *vec);

#endif
