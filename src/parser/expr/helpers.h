#ifndef _HEADER_EXPR_HELPER__
#define _HEADER_EXPR_HELPER__
#include <stdint.h>
#include "../prelude.h"


int cmpexpr(struct Expr *a, struct  Expr *b);
int is_expr_token(enum Lexicon token);
int is_expr(char *line, struct Token tokens[], size_t ntokens);
//size_t get_expr_len(Expr *expr);

int symbol_from_token(const char *line, struct Token token, struct Symbol *value);


#endif
