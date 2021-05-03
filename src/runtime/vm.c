#include "stdlib.h"
#include "stdio.h"
#include "string.h"
#include "vm.h"
#include "../common.h"
#include <bits/stdint-intn.h>
#include <stdint.h>
#include <sys/types.h>

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
    vm->registers.trace=0;

    vm->code=xmalloc(stack_sz);
    vm->code_capacity=stack_sz;
    vm->code_len=0;
    
    vm->code=xmalloc(code_sz);
    vm->code_capacity=code_sz;
    vm->heap_capacity=0;

    vm->stack=xmalloc(heap_sz);
    vm->stack_capacity=heap_sz;
    vm->stack_len=0;
    
}


void run_instruction(enum Opcode op, VM *vm) {
    switch (op) {
        case i32Push:
            memcpy(vm->stack + vm->registers.sp, vm->code + vm->registers.ip, sizeof(int32_t));
            vm->registers.sp += sizeof(int32_t);
            vm->registers.ip += sizeof(int32_t);
            break;

        case i32Add:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            
            memcpy(
                vm->stack + vm->registers.sp - sizeof(int32_t), 
                (int32_t *)(vm->registers.rax + vm->registers.rbx),
                sizeof(int32_t)
            );

            vm->registers.sp -= sizeof(int32_t);
            break;
        
        case i32Div:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            
            memcpy(
                vm->stack + vm->registers.sp - sizeof(int32_t), 
                (int32_t *)(vm->registers.rax / vm->registers.rbx),
                sizeof(int32_t)
            );

            vm->registers.sp -= sizeof(int32_t);
            break;
        
        case i32Sub:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            
            memcpy(
                vm->stack + vm->registers.sp - sizeof(int32_t), 
                (int32_t *)(vm->registers.rax - vm->registers.rbx),
                sizeof(int32_t)
            );

            vm->registers.sp -= sizeof(int32_t);
            break;
        
        case i32Pow:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            
            memcpy(
                vm->stack + vm->registers.sp - sizeof(int32_t), 
                (int32_t *)(vm->registers.rax ^ vm->registers.rbx),
                sizeof(int32_t)
            );

            vm->registers.sp -= sizeof(int32_t);
            break;

        case i32Mul:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            
            memcpy(
                vm->stack + vm->registers.sp - sizeof(int32_t), 
                (int32_t *)(vm->registers.rax * vm->registers.rbx),
                sizeof(int32_t)
            );

            vm->registers.sp -= sizeof(int32_t);
            break;

            break;
        // [1, 2]
        // []
        case i32Modolus:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            
            memcpy(
                vm->stack + vm->registers.sp - sizeof(int32_t), 
                (int32_t *)(vm->registers.rax % vm->registers.rbx),
                sizeof(int32_t)
            );

            vm->registers.sp -= sizeof(int32_t);
            break;

    
        //[2, 0]
        // 2
        // 0
        // [1]
        case i32Gt:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            vm->registers.sp -= sizeof(int32_t)*2;
            vm->code[vm->registers.sp++] = (char)(vm->registers.rax > vm->registers.rbx);
            break;
        
        case i32GtEq:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            vm->registers.sp -= sizeof(int32_t)*2;
            vm->code[vm->registers.sp++] = (char)(vm->registers.rax >= vm->registers.rbx);
            break;
        
        case i32IsEq:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            vm->registers.sp -= sizeof(int32_t)*2;
            vm->code[vm->registers.sp++] = (char)(vm->registers.rax == vm->registers.rbx);
            break;
        
        case i32Lt:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            vm->registers.sp -= sizeof(int32_t)*2;
            vm->code[vm->registers.sp++] = (char)(vm->registers.rax < vm->registers.rbx);
            break;

        case i32LtEq:
            vm->registers.rax=(int32_t)*(vm->stack + vm->registers.sp);
            vm->registers.rbx=(int32_t)*(vm->stack + vm->registers.sp - sizeof(int32_t));
            vm->registers.sp -= sizeof(int32_t)*2;
            vm->code[vm->registers.sp++] = (char)(vm->registers.rax <= vm->registers.rbx);
            break;
        
        case Halt:
            exit(vm->registers.trap);
            break;
        
        case Print:
            printf("print function unimplemented\n");
            break;

        case Ret:
            printf("print ret unimplemented\n");
            break;

        case Goto: // expects i64
            vm->registers.ip = vm->stack[vm->registers.sp];
            vm->registers.sp--;
            break;
        
        // addr
        // 1
        case GotoT:
            if (vm->stack[vm->registers.sp] == 1) {
                vm->registers.sp--;
                vm->registers.ip = vm->stack[vm->registers.sp];
            }
            break;
        
        case GotoF:
            if (vm->stack[vm->registers.sp] == 0) {
                vm->registers.sp--;
                vm->registers.ip = vm->stack[vm->registers.sp];
            }
            break;
        
        case ISANop:
            break;

        default:
            printf("unrecgonized opcode %d", (int)op);
    }
}

const char * popcode(enum Opcode op) {
    switch (op) {
        case Pop: return "pop";
        case i32Push: return "i32push";
        case i32Add: return "i32add";
        case i32Div: return "i32div";
        case i32Sub: return "i32sub";
        case i32Pow: return "i32pow";
        case i32Mul: return "i32pow";
        case i32Modolus: return "i32modolus";
        case i32Gt: return "i32gt";
        case i32GtEq: return "i32gteq";
        case i32IsEq: return "i32iseq";
        case i32Lt: return "i32lt";
        case i32LtEq: return "i32lteq";
        case Halt: return "halt";
        case Print: return "print";
        case Ret: return "ret";
        case Goto: return "goto";
        case GotoT:return "goto_t";
        case GotoF: return "goto_f";
        case ISANop: return "nop";
        default: return "UNKNOWN";
    }
}

int operands_size(enum Opcode op) {
    switch (op) {
        case Pop: return 1;
        case i32Push: return sizeof(int)*2;
        case i32Add: return sizeof(int)*2;
        case i32Div: return sizeof(int)*2;
        case i32Sub: return sizeof(int)*2;
        case i32Pow: return sizeof(int)*2;
        case i32Mul: return sizeof(int)*2;
        case i32Modolus: return sizeof(int)*2;
        case i32Gt: return sizeof(int)*2;
        case i32GtEq: return sizeof(int)*2;
        case i32IsEq: return sizeof(int)*2;
        case i32Lt: return sizeof(int)*2;
        case i32LtEq: return sizeof(int)*2;
        case Halt: return 0;
        case Print: return sizeof(size_t);
        case Ret: return sizeof(size_t);
        case Goto: return sizeof(size_t);
        case GotoT:return sizeof(size_t)+sizeof(char);
        case GotoF: return sizeof(size_t)+sizeof(char);
        case ISANop: return 0;
        default: return -1;
    }
}


void run_vm(VM *vm) {

    while(vm->code_len >= vm->registers.ip) {
        enum Opcode opcode = vm->code[vm->registers.ip];
        run_instruction(opcode, vm);
        
        if (vm->registers.trace > 0)
            printf("%d\t%s", vm->registers.ip, popcode(opcode));
            
        vm->registers.ip++;
    }
}