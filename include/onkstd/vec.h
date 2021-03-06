#ifndef _HEADER__VEC__
#define _HEADER__VEC__

#include <stdint.h>

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
    uint16_t clamp;
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

//TODO: add vector expanding
//TODO: add vector slicing concat
void onk_vec_clamp(struct onk_vec_t *vec, uint16_t max);

void onk_vec_reset(struct onk_vec_t *vec);
int8_t onk_vec_pop(struct onk_vec_t *vec, void * dest);
int8_t onk_vec_free(struct onk_vec_t *vec);
void onk_vec_clear(struct onk_vec_t *vec);

void * onk_vec_copy(struct onk_vec_t *dest, const struct onk_vec_t *src);
void * onk_vec_deep_copy(struct onk_vec_t *dest, const struct onk_vec_t *src);
#endif
