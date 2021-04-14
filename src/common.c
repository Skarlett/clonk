#include <stdlib.h>
// dumb rewrite of xmalloc
void * xmalloc(size_t size) {
  void *p;

  p = malloc(size);
  if (p == NULL) {
    printf ("Not enough memory [%d bytes]\n", (int)size);
    exit(1);
  }
  return p;
}