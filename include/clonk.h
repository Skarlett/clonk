#ifndef __CLONK__
#define __CLONK__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils/vec.h"

#define ONK_VERSION "0.0.2"
#define ONK_INCLUDE_TESTS 0 // -DINCLUDE_TESTS 1


// Macro for checking bitness (safer macros borrowed from
// https://www.fluentcpp.com/2019/05/28/better-macros-better-flags/)
#define SYS_ARCH( X ) SYS_ARCH_PRIVATE_DEFINITION_##X()
// Bitness checks borrowed from
// https://stackoverflow.com/a/12338526/201787
#if _WIN64 || ( __GNUC__ && __x86_64__ )
#    define SYS_ARCH_PRIVATE_DEFINITION_64() 1
#    define SYS_ARCH_PRIVATE_DEFINITION_32() 0
#    define SYS_ARCH_IF_64_BIT_ELSE( x64, x86 ) (x64)
#    define onk_str_to_isize strtoll
#    define onk_str_to_usize strtoull
#    define onk_isize_sz sizeof(int64_t)
#    define onk_usize_sz sizeof(uint64_t)
#elif _WIN32 || __GNUC__
#    define SYS_ARCH_PRIVATE_DEFINITION_64() 0
#    define SYS_ARCH_PRIVATE_DEFINITION_32() 1
#    define SYS_ARCH_IF_64_BIT_ELSE( x64, x86 ) (x86)
#    define onk_str_to_isize strtol
#    define onk_str_to_usize strtoul
#    define onk_isize_sz sizeof(int32_t)
#    define onk_usize_sz sizeof(uint32_t)
#else
#    define onk_str_to_isize strtol
#    define onk_str_to_usize strtoul
#    define onk_isize_sz sizeof(int16_t)
#    define onk_usize_sz sizeof(uint16_t)
#endif

#define onk_nop

/* 64kb */
#define ONK_MAX_INPUT_FILE_SZ 65535

#define ONK_EOF 0

/* error buffer size */
#define ONK_ERR_BUF_SZ 128

/* operator stack size */
#define ONK_PARSER_OP_BUF_SZ 2048

typedef uint16_t onk_bufsz_t;

#if SYS_ARCH(64)
typedef uint64_t usize;
typedef int64_t isize;

#elif SYS_ARCH(86)
typedef uint32_t usize;
typedef int32_t isize;

#else
typedef uint16_t usize;
typedef int16_t isize;

#endif



#endif
