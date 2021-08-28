#ifndef _HEADER_EXPR_HELPER__
#define _HEADER_EXPR_HELPER__
#include <stdint.h>

int cmpexpr(struct Expr *a, struct  Expr *b);
int is_expr_token(enum Lexicon token);
int is_expr(char *line, struct Token tokens[], size_t ntokens);
size_t get_expr_len(Expr *expr);


#endif
