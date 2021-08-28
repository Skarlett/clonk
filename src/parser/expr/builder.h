#ifndef _HEADER__EXPR_BUILD__
#define _HEADER__EXPR_BUILD__
#include "expr.h"
void build_not_expr_T(Expr *expr, Expr *lhs, Expr *target);
void build_bin_expr_T(Expr *expr, Expr *lhs, Expr *rhs, enum BinOp op);
void build_int_value_T(Expr *expr, int value);
void build_str_value_T(Expr *expr, char * str);
void build_var_T(Expr *expr, char * var_name);
void build_fn_call_T(Expr *expr, char *name, Expr *args[], short unsigned int argc);
#endif
