#ifndef _HEADER__LEXER_HELPER__
#define _HEADER__LEXER_HELPER__

const char * ptoken(enum Lexicon t);
inline enum Lexicon invert_brace_type(enum Lexicon token);
inline int8_t is_fncall(enum Lexicon token[2]);
inline int8_t is_cmp_operator(enum Lexicon compound_token);
inline int8_t is_bin_operator(enum Lexicon compound_token);
inline int8_t is_close_brace(enum Lexicon token);
inline int8_t is_open_brace(enum Lexicon token); 
inline int8_t is_data(enum Lexicon token);
inline int8_t is_utf(char ch);

int8_t is_keyword(enum Lexicon token);
int8_t is_balanced(struct Token tokens[], unsigned long int ntokens);
int8_t is_balanced_by_ref(struct Token *tokens[], unsigned long int ntokens);
#endif