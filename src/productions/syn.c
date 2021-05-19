
/*---------------------------------------
// synthesizer
//   maintains sanity by constructing some rules
// in which the interpreter expects.
// the comments below give psuedo code snippets
// expressing what each segment attempts to achieve.
//     `conditional` + `block`
// the example above, would be
// ConditionalStatement and then a BlockStatement would follow.
//---------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include "../parser/lexer.h"
#include "../parser/expr.h"
#include "../parser/ast.h"
#include "syn.h"

int synthesize_expr(Expr *expr) {
    if (expr->type == UndefinedExprT)
        return -1;

    else if (expr->type == BinExprT) {
        if (synthesize_expr(expr->inner.bin.lhs) == -1
            || synthesize_expr(expr->inner.bin.rhs) == -1
            || expr->inner.bin.op != BinaryOperationNop) {
            return -1;
        }
    }

    return 0;
}

int synthesize_block(BlockStatement *block, int ret_allowed){
    for (size_t i=0; block->length > i; i++) {
        
        if (block->statements[i]->type == Undefined) {
            printf("got undefined statement");
            return -1;
        }

        else if (block->statements[i]->type == Block && synthesize_block(block->statements[i]->internal_data, 1) == -1) {
            return -1;
        }
        
        else if (block->statements[i]->type == Condition) {
            if (synthesize_expr(((ConditionalStatement *)block->statements[i]->internal_data)->expr) == -1)
                return -1;
        }

        // return wasn't allowed there
        else if (block->statements[i]->type == Return && !ret_allowed) {
            printf("ret not allowed outside of function body");
            return -1;
        }

        // `conditional`  + `block`
        // `define` + `block`
        if (block->statements[i]->type == Condition || block->statements[i]->type == Define) {
            if (block->statements[i++]->type != Block) {
                printf(
                    "`%s` requires a block to fllow its declaration\n",
                    pstmt_type(block->statements[i]->type)
                );
                return -1;
            }
        }
 
    }
    return 0;
}

int synthesize(BlockStatement *block) {
    return synthesize_block(block, 0);
}