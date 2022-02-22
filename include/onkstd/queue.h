#ifndef __QUEUE__HEADER__
#define __QUEUE__HEADER__

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*
 * FONK_IF_TOKENO data
 */
struct onk_open_queue_t {
    void * base;

    /* denotes the oldest element in the fifo*/
    uint8_t tail;

    uint8_t size;
    uint8_t nitems;

    uint16_t item_sz;
};


void onk_init_queue8(
    struct onk_open_queue_t *queue,
    void * buffer,
    uint16_t item_sz,
    uint8_t capacity
);

/**
 * Push Openinto circular buffer.
 *
 * @param self:
 *
 * README: Copies @param "item" into buffer
 */
void onk_queue8_push(struct onk_open_queue_t *self, void * item);
void * onk_queue8_head(const struct onk_open_queue_t *self);
void * onk_queue8_tail(const struct onk_open_queue_t *self);


/*
 * Get previous tokens up to N tokens
 * implemented as circular buffer.
*/
void * onk_queue8_prev(
    const struct onk_open_queue_t *self,
    uint8_t i
);

#endif
