#ifndef _HEADER__COMMON__
#define _HEADER__COMMON__

#define VERSION "0.0.1"
#define INCLUDE_TESTS 0 // -DINCLUDE_TESTS 1

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Macro for checking bitness (safer macros borrowed from 
// https://www.fluentcpp.com/2019/05/28/better-macros-better-flags/)
#define SYS_ARCH( X ) SYS_ARCH_PRIVATE_DEFINITION_##X()
#define nop
// Bitness checks borrowed from https://stackoverflow.com/a/12338526/201787
#if _WIN64 || ( __GNUC__ && __x86_64__ )
#    define SYS_ARCH_PRIVATE_DEFINITION_64() 1
#    define SYS_ARCH_PRIVATE_DEFINITION_32() 0
#    define SYS_ARCH_IF_64_BIT_ELSE( x64, x86 ) (x64)
#    define strtoisize strtoll
#    define strtousize strtoull
#elif _WIN32 || __GNUC__
#    define SYS_ARCH_PRIVATE_DEFINITION_64() 0
#    define SYS_ARCH_PRIVATE_DEFINITION_32() 1
#    define MYPROJ_IF_64_BIT_ELSE( x64, x86 ) (x86)
#    define str_to_isize strtol
#    define str_to_usize strtoul
#else
#    error "64/32bit machines only"
#endif

#if SYS_ARCH(64)
/* pointer sized unsigned integer*/
typedef uint64_t usize;

/* pointer sized signed integer*/
typedef int64_t isize;

#else
/* pointer sized unsigned integer*/
typedef uint32_t usize;

/* pointer sized signed integer*/
typedef int32_t isize;
#endif


#endif
