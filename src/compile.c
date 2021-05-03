#include "parser/lexer.h"
#include "parser/expr.h"
#include "runtime/vm.h"

void i32pack(int a, char *code) {
    memcpy(code, &a, sizeof(int));
}

int i32unpack(char *code) {
    int i;
    memcpy(&i, code, sizeof(int));
    return i;
}


void compile_expr(Expr *expr, char *code, size_t code_capacity, size_t ncode) {
    if (expr->type == UniExprT) {        
        UniExpr* uni = expr->inner_data;
        if (uni->op == UniValue) {
            Symbol *sym = uni->inner;
        
            if (sym->tag==ValueTag) {
                Value *val = uni->inner;
                if (val->datatype == IntT) {
                    *code[ncode] = (char)i32Push;
                    ncode++;
                    i32pack((int)val->data_ptr, code + ncode)
                    
                }

                else if (sym->tag == VariableTag) {

                }
        
            }
            
        }
    }
}