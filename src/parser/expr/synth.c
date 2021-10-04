#include <sys/types.h>
#include <stdint.h>
#include "../../prelude.h"
#include "../lexer/lexer.h"
#include "../lexer/helpers.h"

int8_t simple_validate_postfix(struct Token *tokens[], usize ntokens){
    // if more than one operand - then n-1 operators will be

    uint8_t operands = 0,
            operators = 0;
    
    
    if (ntokens == 1)
        return 1;
    
    else if (ntokens <= 0)
        return -1;
    
    for (usize i=0; ntokens > i; i++) {

    }
    

}


/*
    https://stackoverflow.com/a/789867/5249205
*/
int8_t ctr_validate_postfix(struct Token tokens[], usize ntokens, masks) {
    isize ctr=0;

    for (usize i=0; ntokens > i; i++) {
        if (is_data(tokens[i]->type)) {
            ctr += 1;
        }
        else if (is_bin_operator(tokens[i]->type)) {
            ctr -= 2;
            if (ctr > 0) return 0;
            ctr += 1;
        }
        else if ()
    }

    return ctr == 1;
}

int8_t is_valid_postfix(struct Token *tokens[], usize ntokens) {

}



int8_t missing_operand(enum Lexicon token) {
    return is_bin_operator(token)
    || is_open_brace(token)
    || is_close_brace(token)
    || token == COMMA;
}

int8_t synth_expr(struct Token tokens[], usize ntokens) {
    for(usize i=0; ntokens > i; i++) {
        if (is_bin_operator(tokens[i].type) || tokens[i].type == COMMA) {
            if (i == 0) return 0;

            else if (missing_operand(tokens[i-1].type) || missing_operand(tokens[i+1].type))
                return 0;
        }
        else if ()
    }

    return 1;
}