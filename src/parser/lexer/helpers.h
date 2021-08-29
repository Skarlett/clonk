#ifndef _HEADER__LEXER_HELPER__
#define _HEADER__LEXER_HELPER__

enum Lexicon invert_brace_type(enum Lexicon token);
const char * ptoken(enum Lexicon t);
int is_cmp_operator(enum Lexicon compound_token);
int is_bin_operator(enum Lexicon compound_token);
int is_close_brace(enum Lexicon token);
int is_open_brace(enum Lexicon token); 
int is_data(enum Lexicon token);
int is_balanced(struct Token tokens[], unsigned long int ntokens);
int is_balanced_by_ref(struct Token *tokens[], unsigned long int ntokens);

#endif