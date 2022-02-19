#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include "utils/queue.h"

void init_queue8(
    struct OpenQueue8_t *queue,
    void * buffer,
    uint16_t item_sz,
    uint8_t capacity
){
    queue->base = buffer;
    queue->size = capacity;
    queue->nitems = 0;
    queue->tail = 0;
    queue->item_sz = item_sz;
}

/*
** Push value into circular buffer.
**
** README: Copies @param "item" into buffer
*/
void queue8_push(
    struct OpenQueue8_t *self,
    void *item
){

    memcpy(
        &self->base + (self->tail * self->item_sz), item, self->item_sz
    );

    /* adjust head */
    if (self->nitems == self->size && self->tail == self->nitems)
        self->tail = 0;
    else
    {
        self->tail += 1;
        self->nitems += 1;
    }
}

/*
** Get last item inserted in buffer
**
*/
void * queue8_head(
    const struct OpenQueue8_t *self
){
    return self->base + ((self->tail || 1) - 1) * self->item_sz;
}

/*
** get last item in buffer
**
*/
void * queue8_tail(
    const struct OpenQueue8_t *self
){

    /* if not circular yet */
    if (self->size > self->nitems)
        return self->base;

    /* if circular */
    else if (self->size == self->nitems)
        return &self->base[self->tail];

    return 0;
}


/*
 * steps back N steps in circular buffer
 *
*/
void * queue8_step_back(
    const struct OpenQueue8_t *self,
    uint8_t n
){
    uint16_t working_int;

    if (n == 0
        || n >= self->size
        || n >= self->nitems
        || self->nitems == 0)
        return self->base;

    if (n > self->tail && self->nitems >= n )
      return self->base + self->item_sz * (n - self->tail);

    else
      return self->base + self->item_sz * (self->tail - n);
}
