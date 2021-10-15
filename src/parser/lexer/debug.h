#ifndef _HEADER__LEXER_DBG__
#define _HEADER__LEXER_DBG__
#include <stdint.h>
#include "../../prelude.h"
#include "lexer.h"

const char * ptoken(enum Lexicon t);

char invert_brace_char(char brace);

int8_t sprintf_token_slice(
    const struct Token tokens[],
    usize ntokens,
    char * output,
    usize output_sz    
);

int8_t sprintf_token_slice_by_ref(
    const struct Token *tokens[],
    usize ntokens,
    char * output,
    usize output_sz    
);

int8_t sprintf_lexicon_slice(
    const enum Lexicon tokens[],
    usize ntokens,
    char * output,
    usize output_sz    
);

int8_t sprint_src_code(
    char * output,
    usize output_sz,
    const char * source,
    const struct Token *token
);

char brace_as_char(enum Lexicon tok);
#endif