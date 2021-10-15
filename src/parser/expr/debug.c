#include <stdint.h>
#include <stdio.h>

#include "../lexer/lexer.h"
#include "expr.h"
#include "helpers.h"
#include "../lexer/debug.h"

const char * print_datatype(enum DataType t) {
    switch (t) {
        case StringT: return "string";
        case IntT: return "integer";
        case NullT: return "null";
        default: return "undefined";
    }
}

const char * print_bin_operator(enum Operation t) {
    switch (t) {
        case Add: return "addition";
        case Sub: return "subtract";
        case Multiply: return "multiply";
        case Divide: return "divide";
        case Modolus: return "modolus";
        case Pow: return "power";
        case And: return "and";
        case Or: return "or";
        case GtEq: return "gteq";
        case Gt: return "gt";
        case Lt: return "lt";
        case LtEq: return "lteq";
        case IsEq: return "iseq";
        case NotEq: return "noteq";
        default: return "unknown";
    }
}

const char * p_bin_operator_sym(enum Operation t) {
    switch (t) {
        case Add: return "+";
        case Sub: return "-";
        case Multiply: return "*";
        case Divide: return "/";
        case Modolus: return "%";
        case Pow: return "^";
        case And: return "&&";
        case Or: return "||";
        case GtEq: return "<=";
        case Gt: return "<";
        case Lt: return ">";
        case LtEq: return ">=";
        case IsEq: return "==";
        case NotEq: return "!=";
        case Access: return ".";
        default: return "unknown";
    }
}


/*
 * 
 */
usize pprint_postfix(
    char *output, 
    usize output_sz,
    const char *line,
    const struct Token *tokens[],
    usize ntokens
){
    usize i=0;
    struct BorderItem borders[512];
    
    usize borders_len = 0;
    usize borders_ctr = 0;

    usize str_ctr = 0;

    if (mk_group_borders(borders, 512, &borders_len, tokens, ntokens) == -1
        || chk_group_overlap(borders, borders_len) == 1)
        return -1;
    

    for (i=0; ntokens > i; i++) {
        
        if (str_ctr + 2 >= output_sz)
            return -1;
        
        else if (borders[borders_ctr].start == i){
            output[str_ctr] = borders[borders_ctr].brace;
            output[str_ctr + 1] = ' ';
            str_ctr += 2;
        }

        else if (borders[borders_ctr].end == i) {
            output[str_ctr] = invert_brace_char(borders[borders_ctr].brace);
            output[str_ctr + 1] = ' ';
            str_ctr += 2;
            
            /* skip over group token */
            continue;
        }
        
        str_ctr += sprint_src_code(output + str_ctr, output_sz, line, tokens[i]);
        output[str_ctr + 1] = ' ';
        str_ctr += 2;
    }

    return str_ctr;
}

void draw_token_error_at(const char * line, struct Token *token) {
    printf("'%s'\n", line);
    printf("-");
    for (int i = 0; token->start > i; i++) {
        printf("-");
    }
    
    printf("^\n");
}
