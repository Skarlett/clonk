#include <stdlib.h>
#include <stdio.h>
// dumb rewrite of xmalloc
void * xmalloc(size_t size) {
  void *p;

  p = malloc(size);
  if (p == NULL) {
    printf("Not enough memory [%d bytes]\n", (int)size);
    exit(1);
  }
  return p;
}

// // dumb rewrite of
// void * xrealloc(void *ptr, size_t size) {
//   void *tmp;
//   if (ptr != 0) {
//     void * maybe = realloc(tmp, size);
//     if (tmp == NULL) {
//         free(ptr);
//         printf ("Not enough memory [%d bytes]\n", (int)size);
//         exit(1);
//     }
//     else {
//         ptr=tmp; //maybe? test it later ig
//     }
//   }
// }


void tab_print(short unsigned indent) {
    for (short unsigned i=0; indent > i; i++)
            printf("\t");
}