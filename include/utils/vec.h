#ifndef _HEADER__VEC__
#define _HEADER__VEC__
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

struct onk_vec_t {
    void *base;
    void *head;
    uint16_t type_sz;
    size_t capacity;
    size_t len;

    uint8_t state;
};

int8_t onk_vec_init(struct onk_vec_t *vec, size_t capacity, size_t type_sz);
int8_t onk_vec_realloc(struct onk_vec_t *vec);
void * onk_vec_push(struct onk_vec_t *vec, const void *src);
int8_t onk_vec_free(struct onk_vec_t *vec);
const void * onk_vec_head(const struct onk_vec_t *vec);
int8_t onk_vec_pop(struct onk_vec_t *vec, void * dest);
int8_t onk_vec_clear(struct onk_vec_t *vec);

#endif
