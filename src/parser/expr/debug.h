#ifndef _HEADER__EXPR_DBG__
#define _HEADER__EXPR_DBG__

#include "expr.h"
//#include "../lexer/lexer.h"

//void ptree(struct Expr *expr);
void draw_token_error_at(char * line, struct Token *token);
//int print_expr(struct Expr *expr, short unsigned indent);
const char * p_bin_operator_sym(enum BinOp t);
const char * print_bin_operator(enum BinOp t);
const char * print_expr_t(enum ExprType t);
const char * print_datatype(enum DataType t);
#endif