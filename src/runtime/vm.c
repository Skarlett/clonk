#include "stdlib.h"
#include "stdio.h"
#include "vm.h"
#include "../common.h"

void init_vm(VM *vm,
    size_t stack_sz,
    size_t heap_sz,
    size_t code_sz,
    size_t i32_const_sz    
){
    vm->registers.fp=0;
    vm->registers.sp=0;
    vm->registers.ip=0;
    vm->registers.rp=0;
    vm->registers.trap=0;

    vm->code=xmalloc(stack_sz);
    vm->code_capacity=stack_sz;
    vm->code_len=0;
    
    vm->code=xmalloc(code_sz);
    vm->code_capacity=code_sz;
    vm->heap_capacity=0;

    vm->stack=xmalloc(heap_sz);
    vm->stack_capacity=heap_sz;
    vm->stack_len=0;
    
    
    vm->consts.i32_const=xmalloc(i32_const_sz);
}

void run_instruction(enum Opcode op, VM *vm) {
    switch (op) {
        case Pop:
            vm->registers.sp--;    
            break;

        case i32Const:
            vm->stack[vm->registers.sp] = vm->consts.i32_const[vm->code[vm->registers.ip++]];
            vm->registers.sp++;
            break;

        case i32Add:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] + vm->stack[vm->registers.sp];
            break;
        
        case i32Div:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] / vm->stack[vm->registers.sp];
            break;
        
        case i32Sub:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] - vm->stack[vm->registers.sp];
            break;
        
        case i32Pow:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] ^ vm->stack[vm->registers.sp];
            break;

        case i32Mul:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] * vm->stack[vm->registers.sp];
            break;
        // [1, 2]
        // []
        case i32Modolus:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] % vm->stack[vm->registers.sp];
            break;
    
        //[2, 0]
        // 2
        // 0
        // [1]

        case i32Gt:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] > vm->stack[vm->registers.sp];
            break;
        
        case i32GtEq:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] >= vm->stack[vm->registers.sp];
            break;
        
        case i32IsEq:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] == vm->stack[vm->registers.sp];
            break;
        
        case i32Lt:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] < vm->stack[vm->registers.sp];
        
        case i32LtEq:
            vm->stack[vm->registers.sp] = vm->stack[vm->registers.sp--] <= vm->stack[vm->registers.sp];

        case Halt:
            exit(vm->registers.trap);
            break;
        
        case Print:
            printf("print function unimplemented\n");
            break;

    }

}

void run_vm(VM *vm) {
    enum Opcode opcode = vm->code[vm->registers.ip];
    vm->registers.ip++;


}