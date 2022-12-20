#include <stdint.h>
#include <string.h>

/* First in First Out Fixed-sized array */
struct FifoArr {
    void * base;
    void * head;

    uint16_t ctr;
    uint16_t len;

    uint16_t capacity;
    uint16_t type_sz;
};

int8_t filoarr_init(
    struct FifoArr *self,
    void * initalized_base,
    uint16_t capacity,
    uint16_t size
)
{
    self->len = 0;
    self->ctr = 0;

    self->base = initalized_base;
    self->head = initalized_base;
    self->capacity = capacity;
    self->type_sz = size;

    if(self->capacity % 2 != 0)
      return -1;
    
    return 0;
}

void * filoarr_push(struct FifoArr *self, void * item) {
    void * ret = self->head;    
    
    if (self->capacity > self->len)
      self->len += 1;
    
    if (self->ctr >= self->capacity) {
        self->ctr = 0;
        self->head = self->base;
    }
    self->ctr += 1;

    memcpy(ret, item, self->type_sz);
    self->head += self->type_sz;

    return ret;
}

void * filoarr_idx_last(struct FifoArr *self, uint16_t idx)
{
  if (idx > self->capacity || self->len > idx)
    return 0;
  
  else if (idx == 0)
    return self->head - self->type_sz;
  
  else if(self->ctr >= idx) {
      return self->base + (self->type_sz * (self->ctr - idx) - self->type_sz);
  }

  else if(idx > self->ctr && self->len > idx) {
      return self->base + (self->type_sz * (self->ctr - idx) - self->type_sz);
  }

}