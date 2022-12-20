#include <stdint.h>
#include "clonk.h"
#include "vec.h"

struct CB_Frame {
  uint16_t len;
  void * ptr;
};


/*
internally carry a buffer of pointers, 
but treat them as one block of memory
*/
struct ChunkedBufferVec {
  
  // Vec<void *>
  struct onk_vec_t buffer;
  uint16_t type_sz;
};

void * cbv_get(struct ChunkedBufferVec *arr, uint16_t idx)
{
  uint16_t i;
  uint16_t accumulator = 0;

  struct CB_Frame *entry;

  for (i=0; arr->buffer.len > i; i++) {
      entry = arr->buffer.base;
      if(entry[i].len > idx) {
          entry[i].ptr + (arr->buffer.type_sz * (idx - accumulator));
             
          //accumulator += entry[i].len - 1;
      }
      else
      {
        if(entry[i].len > 0)
          accumulator += entry[i].len;        
      }
  }
}

struct ChunkedBufferArr {
  // {void *, ...}
  struct CB_Frame *buffer;
  uint16_t type_sz;
};

struct CBIter {
  // {void *, ...}
  struct CB_Frame *buf;
  uint16_t type_sz;
  uint16_t len;
  /*
  */
  uint16_t *i;
  uint16_t k;
};


struct CBEntry {
  uint16_t i;
  uint16_t k;
};

void * foo(void ** ptr, uint16_t ty_sz)
{
  bool run = true;
  uint16_t i = 0;
  while(run){
    
    for (uint16_t i2 = 0;; i2++)
      if(  *(uint16_t *)(ptr[i] + (ty_sz * i2)) == 0)
      
  }
  return 0;
  
}
bool cbv_foreach(const struct CBIter *state, uint16_t i) {
   state->buffer
}

// struct CBEntry cbv_foreach(struct CBIter *arr, uint16_t idx)
// {
//   uint16_t i, k;
//   uint16_t accumulator = 0;

//   struct CB_Frame *entry;
  
//   entry = arr->buffer[i];

// //   for (i=0; arr->buffer.len > i; i++) {
// //       for (k=0 ;; k++);
// //   }
      
// }
