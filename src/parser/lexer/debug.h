#ifndef _HEADER__LEXER_DBG__
#define _HEADER__LEXER_DBG__
#include <stdint.h>
#include "../../prelude.h"
#include "lexer.h"

int8_t sprintf_token_slice(
    const struct Token tokens[],
    uint16_t ntokens,
    char * output,
    uint16_t output_sz    
);

int8_t sprintf_token_slice_by_ref(
    const struct Token *tokens[],
    uint16_t ntokens,
    char * output,
    uint16_t output_sz    
);

int8_t sprintf_lexicon_slice(
    const enum Lexicon tokens[],
    uint16_t ntokens,
    char * output,
    uint16_t output_sz    
);

int8_t sprint_src_code(
    char * output,
    uint16_t output_sz,
    uint16_t *nbytes,
    const char * source,
    const struct Token *token
);

//char brace_as_char(enum Lexicon tok);
#endif
