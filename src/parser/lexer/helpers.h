#ifndef _HEADER__LEXER_HELPER__
#define _HEADER__LEXER_HELPER__

#include <stdint.h>
#include "../../prelude.h"
#include "lexer.h"

enum Lexicon invert_brace_tok_ty(enum Lexicon token);

int8_t is_fncall(struct Token tokens[], usize ntokens);
int8_t is_cmp_operator(enum Lexicon compound_token);
int8_t is_bin_operator(enum Lexicon compound_token);

int8_t is_close_brace(enum Lexicon token);
int8_t is_open_brace(enum Lexicon token); 

int8_t is_data(enum Lexicon token);
int8_t is_utf(char ch);

int8_t is_keyword(enum Lexicon token);
int8_t is_balanced(struct Token tokens[], usize ntokens);
int8_t is_balanced_by_ref(struct Token *tokens[],usize ntokens);

int8_t is_num_negative(struct Token *token);

#endif
