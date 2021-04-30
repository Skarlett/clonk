

//synthesizer
// maintains sanity by constructing some rules
// in which the interpreter expects

#include <stdlib.h>
#include "lexer.h"
#include "expr.h"
#include "ast.h"

int synthesize_block(BlockStatement *block, int ret_allowed){
    for (size_t i=0; block->length > i; i++) {
        if (block->statements[i]->type == Block) 
            synthesize_block(block->statements[i]->internal_data, 1);
        
        // `if`|`else`|`elif`  + `block`
        // `define` + `block`
        else if (block->statements[i]->type == Condition || block->statements[i]->type == Define) {
            // check next statement is block
            i++;
            if (block->statements[i]->type != Block) {
                return -1;
            }
        }

        // !`block`.contains(`return`)
        // `define` + `block`.contains(`return`)?
        else if (block->statements[i]->type == Return && !ret_allowed)
            return -1; 
    }
}

int synthesize(BlockStatement *block) {
    return synthesize_block(block, 0);
}