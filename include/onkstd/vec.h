#ifndef _HEADER__VEC__
#define _HEADER__VEC__

#include <stdint.h>

enum onk_vec_mode_t {
  onk_vec_mode_uninit = 0,
  onk_vec_mode_free   = 1,
  onk_vec_mode_alloc_heap  = 2,
  onk_vec_mode_alloc_stack = 3
};

struct onk_vec_t {
    void *base;
    int state;
    uint16_t type_sz;
    uint16_t capacity;
    uint16_t len;

    uint16_t inc;
    uint16_t clamp;
};

const void * onk_vec_head(const struct onk_vec_t *vec);

void onk_vec_alloc_heap(
    struct onk_vec_t *vec,
    uint16_t capacity,
    uint16_t type_sz
);

void onk_vec_alloc_stk(
    struct onk_vec_t *vec,
    void *stack_ptr,
    uint16_t capacity,
    uint16_t type_sz
);

void onk_vec_init(struct onk_vec_t *vec);

int8_t onk_vec_realloc(
    struct onk_vec_t *vec,
    uint16_t inc
);

void * onk_vec_push(
    struct onk_vec_t *vec,
    const void *src
);

//TODO: add vector expanding
//TODO: add vector slicing concat
void onk_vec_clamp(struct onk_vec_t *vec, uint16_t max);

void onk_vec_reset(struct onk_vec_t *vec);
int8_t onk_vec_pop(struct onk_vec_t *vec, void * dest);
void onk_vec_free(struct onk_vec_t *vec);
void onk_vec_clear(struct onk_vec_t *vec);

void onk_vec_move(struct onk_vec_t *dest, struct onk_vec_t *src);
void onk_vec_new(struct onk_vec_t *vec, uint16_t cap, uint16_t type_sz);
void onk_vec_new_stk(
    struct onk_vec_t *vec,
    void *stack_ptr,
    uint16_t capacity,
    uint16_t type_sz
);

void * onk_vec_copy(struct onk_vec_t *dest, const struct onk_vec_t *src);
void * onk_vec_deep_copy(struct onk_vec_t *dest, const struct onk_vec_t *src);
void onk_vec_deep_copy_resize(
    struct onk_vec_t *dest,
    struct onk_vec_t *src
);

#endif
