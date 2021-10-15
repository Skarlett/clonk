#include <stdint.h>
#include <string.h>
#include "../lexer/lexer.h"
#include "../lexer/debug.h"
#include "expr.h"
#include "helpers.h"
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include "../../prelude.h"


enum Operation operation_from_token(enum Lexicon t){
    switch (t) {
        case ADD: return Add;
        case SUB: return Sub;
        case MUL: return Multiply;
        case DIV: return Divide;
        case MOD: return Modolus;
        case POW: return Pow;
        case AND: return And;
        case OR: return Or;
        case ISEQL: return IsEq;
        case ISNEQL: return NotEq;
        case GTEQ: return GtEq;
        case LTEQ: return LtEq;
        case LT: return Lt;
        case GT: return Gt;
        case DOT: return Access;
        default: return UndefinedOp;
    }
}

int8_t mk_group_borders(
    struct BorderItem borders[],
    usize border_sz,
    usize *border_ctr,
    const struct Token *tokens[],
    usize ntokens
){
    if (*border_ctr > border_sz)
        return -1;
    
    for (usize i=0; ntokens > i; i++)
    {
        if (tokens[i]->type == GROUPING)
        {
            borders[*border_ctr].start = i - tokens[i]->end;
            borders[*border_ctr].end = tokens[i]->end;
            borders[*border_ctr].brace = brace_as_char(tokens[i]->type);
            *border_ctr += 1;
        }
    }
    return *border_ctr > 0;
}


int8_t chk_group_overlap(
    const struct BorderItem borders[],
    usize border_len
){
    
    usize last[2] = {0, 0};

    for (usize i=0; border_len > i; i++)
    {
        if (last[0] >= borders[i].end
            || last[1] >= borders[i].start
            || last[0] >= borders[i].start
            || last[1] >= borders[i].end)
            return 1;
        
        last[0] = borders[i].start;
        last[1] = borders[i].end;
    }
    return 0;
}