#ifndef _HEADER__COMMON__
#define _HEADER__COMMON__

#define VERSION "0.0.1"
#define INCLUDE_TESTS 0 // -DINCLUDE_TESTS 1

#include <stdlib.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef size_t usize;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
typedef int isize;

#define TRUE 1;
#define FALSE 0;

void * xmalloc(usize size);
void * xrealloc(usize size);
void * tab_print(usize size);

#endif
