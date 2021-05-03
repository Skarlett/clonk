#ifndef _HEADER__VM__
#define _HEADER__VM__

#include <stdlib.h>
#include <sys/types.h>

enum Opcode {
    ISANop,
    i32Const,
    
    i32IsEq,
    i32Gt,
    i32Lt,
    i32GtEq,
    i32LtEq,

    i32Add,
    i32Sub,
    i32Div,
    i32Mul,
    i32Modolus,
    i32Pow,

    // StrConst,
    // StrIsEq,

    Store,
    GStore,

    GLoad,
    Load,

    Goto,
    GotoT,
    GotoF,

    Pop,

    Ret,
    
    Halt,

    Print
};



typedef struct VMRegisters {
    size_t ip; // instruction ptr
    size_t sp; // stack ptr
    size_t fp; // frame ptr
    size_t rp; // return addr

    char trap; // trap for error [0-255]
} VMRegisters;


typedef struct Constants {
    ssize_t *i32_const;
    size_t i32_const_capacity;
    size_t i32_const_len;

    // char **str_const;
    // size_t str_const_capacity;
    // size_t str_const_len;

} Constants;

typedef struct VM {
    VMRegisters registers;
    Constants consts;
    
    // okay, so the reason we're choosing `int` as our size default size,
    // is so that we don't have to deal with down/up casting types
    int *stack;
    size_t stack_len;
    size_t stack_capacity;

    char *code;
    size_t code_len;
    size_t code_capacity;

    char *heap;
    size_t heap_len;
    size_t heap_capacity;
    size_t heap_sz;

} VM;

#endif