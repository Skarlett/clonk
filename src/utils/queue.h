#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/*
 * FIFO data
 */
struct OpenQueue8_t {
    void * base;

    /* denotes the oldest element in the fifo*/
    uint8_t tail;

    uint8_t size;
    uint8_t nitems;

    uint16_t item_sz;
};


void init_queue8(
    struct OpenQueue8_t *queue,
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
void queue8_push(struct OpenQueue8_t *self, void * item);
void * queue8_head(const struct OpenQueue8_t *self);
void * queue8_tail(const struct OpenQueue8_t *self);


/*
 * Get previous tokens up to N tokens
 * implemented as circular buffer.
*/
void * queue8_prev(
    const struct OpenQueue8_t *self,
    uint8_t i
);
