#ifndef _HEADER__COMMON__
#define _HEADER__COMMON__

#define VERSION "0.0.1"

#include <stdlib.h>
#include <stdint.h>
#define TRUE 1
#define FALSE 0



void * xmalloc(uint32_t size);
void * xrealloc(uint32_t size);
void * tab_print(uint32_t size);

#endif