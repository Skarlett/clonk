#ifndef _HEADER_EXPR_HELPER__
#define _HEADER_EXPR_HELPER__
#include <stdint.h>
#include "../../prelude.h"
#include "../lexer/lexer.h"
#include "expr.h"

struct BorderItem {
    usize start;
    usize end;

    /*
      The brace should always be the opening type,
      valid input examples: '(', '[', '{'
    */
    char brace;
};

enum Operation operation_from_token(enum Lexicon t);

int8_t chk_group_overlap(
    const struct BorderItem borders[],
    usize border_len
);

int8_t mk_group_borders(
    struct BorderItem borders[],
    usize border_sz,
    usize *border_ctr,
    const struct Token *tokens[],
    usize ntokens
);
#endif
