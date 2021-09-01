#ifndef _HEADER__LEXER_HELPER__
#define _HEADER__LEXER_HELPER__

const char * ptoken(enum Lexicon t);
inline enum Lexicon invert_brace_type(enum Lexicon token);
inline int is_cmp_operator(enum Lexicon compound_token);
inline int is_bin_operator(enum Lexicon compound_token);
inline int is_close_brace(enum Lexicon token);
inline int is_open_brace(enum Lexicon token); 
inline int is_data(enum Lexicon token);
inline int is_utf(char ch);

int is_keyword(enum Lexicon token);
int is_balanced(struct Token tokens[], unsigned long int ntokens);
int is_balanced_by_ref(struct Token *tokens[], unsigned long int ntokens);
#endif