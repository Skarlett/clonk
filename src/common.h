#ifndef _HEADER__COMMON__
#define _HEADER__COMMON__

#define VERSION "0.0.1"

#include <stdlib.h>

#define TRUE 1
#define FALSE 0



void * xmalloc(size_t size);
void * xrealloc(size_t size);
void * tab_print(size_t size);

#endif