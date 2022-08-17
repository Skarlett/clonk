#ifndef _HEADER__VM__
#define _HEADER__VM__

#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

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
    // short cuts
    i32Square,
    i32Inc,
    i32Dec,
    // StrConst,
    // StrIsEq,
    StrCmp,

    Store,
    GStore,

    GLoad,
    Load,

    Goto,
    GotoT,
    GotoF,
    
    Call,
    
    Drop,
    Pop,
    
    Halt,

    Ret,
    Print
};

typedef struct VMRegisters {
    uint64_t rip; // instruction ptr
    uint64_t rsp; // stack ptr
    uint64_t rbp; // stack base ptr (ebp + esp = stack frame)
    

    // caller saves these
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;

    // callee saves these
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;

    short trace;
    short trace_operand;

    unsigned char trap; // trap for error
} VMRegisters;

 
typedef struct VM {
    VMRegisters registers;
    
    // okay, so the reason we're choosing `int` as our size default size,
    // is so that we don't have to deal with down/up casting types
    char *stack;
    uint64_t stack_len;
    uint64_t stack_capacity;

    char *code;
    uint64_t code_len;
    uint64_t code_capacity;

    char *heap;
    uint64_t heap_len;
    uint64_t heap_capacity;
    uint64_t heap_sz;

} VM;

void init_vm(VM *vm,
    uint64_t stack_sz,
    uint64_t heap_sz,
    uint64_t code_sz,
    uint64_t i32_const_sz
);

void run_instruction(enum Opcode op, VM *vm);
const char * popcode(enum Opcode op);
#endif