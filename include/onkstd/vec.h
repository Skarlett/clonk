#ifndef _HEADER__VEC__
#define _HEADER__VEC__
#include "clonk.h"

enum onk_vec_state {
  ONK_VEC_UNINIT,
  ONK_VEC_READY,
};

struct onk_vec_t {
    void *base;

    uint8_t state;
    uint16_t type_sz;
    uint16_t capacity;
    uint16_t len;
    uint16_t max;
    uint16_t inc;
};

const void * onk_vec_head(const struct onk_vec_t *vec);

int8_t onk_vec_init(
    struct onk_vec_t *vec,
    uint16_t capacity,
    uint16_t type_sz);

int8_t onk_vec_realloc(
    struct onk_vec_t *vec,
    uint16_t inc);

void * onk_vec_push(
    struct onk_vec_t *vec,
    const void *src);

//TODO: Add vector clamping
//TODO: add vector expanding
//TODO: add vector slicing concat
int8_t onk_vec_clamp(struct onk_vec_t *vec, uint16_t max);

void onk_vec_reset(struct onk_vec_t *vec);
int8_t onk_vec_pop(struct onk_vec_t *vec, void * dest);
int8_t onk_vec_free(struct onk_vec_t *vec);
void onk_vec_clear(struct onk_vec_t *vec);

#endif
