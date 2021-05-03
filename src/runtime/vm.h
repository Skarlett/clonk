#ifndef _HEADER__VM__
#define _HEADER__VM__

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#define i32 signed

enum Opcode {
    ISANop,
    i32Const,
    
    i32Push,
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
    
    Halt,

    Ret,
    Print
};

typedef struct VMRegisters {
    size_t ip; // instruction ptr
    size_t sp; // stack ptr
    size_t fp; // frame ptr
    size_t rp; // return addr
    
    size_t rax;
    size_t rbx;
    size_t rcx;
    size_t rdx;


    short trace;
    short trace_operand;

    char trap; // trap for error [0-255]
} VMRegisters;


typedef struct VM {
    VMRegisters registers;
    
    // okay, so the reason we're choosing `int` as our size default size,
    // is so that we don't have to deal with down/up casting types
    char *stack;
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

void init_vm(VM *vm,
    size_t stack_sz,
    size_t heap_sz,
    size_t code_sz,
    size_t i32_const_sz    
);

void run_instruction(enum Opcode op, VM *vm);
const char * popcode(enum Opcode op);
#endif