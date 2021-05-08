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
    u_int64_t rip; // instruction ptr
    u_int64_t rsp; // stack ptr
    u_int64_t rbp; // stack base ptr (ebp + esp = stack frame)
    

    // caller saves these
    // NOTE: eax will be used as the return proceedure
    u_int64_t rax;
    u_int64_t rbx;
    u_int64_t rcx;

    // callee saves these
    u_int64_t rdx;
    u_int64_t rsi;
    u_int64_t rdi;

    short trace;
    short trace_operand;

    char trap; // trap for error [0-255]
} VMRegisters;

 
typedef struct VM {
    VMRegisters registers;
    
    // okay, so the reason we're choosing `int` as our size default size,
    // is so that we don't have to deal with down/up casting types
    char *stack;
    u_int64_t stack_len;
    u_int64_t stack_capacity;

    char *code;
    u_int64_t code_len;
    u_int64_t code_capacity;

    char *heap;
    u_int64_t heap_len;
    u_int64_t heap_capacity;
    u_int64_t heap_sz;

} VM;

void init_vm(VM *vm,
    u_int64_t stack_sz,
    u_int64_t heap_sz,
    u_int64_t code_sz,
    u_int64_t i32_const_sz
);

void run_instruction(enum Opcode op, VM *vm);
const char * popcode(enum Opcode op);
#endif