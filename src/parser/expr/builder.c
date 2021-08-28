#include <string.h>
#include "expr.h"

void build_not_expr_T(Expr *expr, Expr *lhs, Expr *target) {
    lhs->type = UniExprT;
    lhs->inner.uni.op = UniValue;
    lhs->inner.uni.interal_data.symbol.tag=ValueTag;
    lhs->inner.uni.interal_data.symbol.inner.value.type=IntT;
    lhs->inner.uni.interal_data.symbol.inner.value.data.integer=0;

    expr->type = BinExprT;
    expr->inner.bin.op = IsEq;
    expr->inner.bin.lhs = lhs;
    expr->inner.bin.rhs = target;
}

void build_bin_expr_T(Expr *expr, Expr *lhs, Expr *rhs, enum BinOp op) {
    expr->type = BinExprT;
    expr->inner.bin.op = op;
    expr->inner.bin.lhs = lhs;
    expr->inner.bin.rhs = rhs;
}

void build_int_value_T(Expr *expr, int value) {
    expr->type = UniExprT;
    expr->inner.uni.op = UniValue;
    expr->inner.uni.interal_data.symbol.tag = ValueTag;
    expr->inner.uni.interal_data.symbol.inner.value.type = IntT;
    expr->inner.uni.interal_data.symbol.inner.value.data.integer = value;
}

void build_str_value_T(Expr *expr, char * str) {
    expr->type = UniExprT;
    expr->inner.uni.op = UniValue;
    expr->inner.uni.interal_data.symbol.tag = ValueTag;
    expr->inner.uni.interal_data.symbol.inner.value.type = StringT;
    expr->inner.uni.interal_data.symbol.inner.value.data.string.ptr = malloc(strlen(str)+1);
    expr->inner.uni.interal_data.symbol.inner.value.data.string.length = strlen(str);
    expr->inner.uni.interal_data.symbol.inner.value.data.string.capacity = strlen(str)+1;
}

void build_var_T(Expr *expr, char * var_name) {
    expr->type = UniExprT;
    expr->inner.uni.op = UniValue;
    expr->inner.uni.interal_data.symbol.tag = VariableTag;
    expr->inner.uni.interal_data.symbol.inner.variable = malloc(strlen(var_name));
}

void build_fn_call_T(Expr *expr, char *name, Expr *args[], short unsigned int argc) {
    expr->type = UniExprT;
    expr->inner.uni.op = UniCall;
    expr->inner.uni.interal_data.fncall.args = args;
    expr->inner.uni.interal_data.fncall.args_capacity = argc;
    expr->inner.uni.interal_data.fncall.args_length = argc;
    expr->inner.uni.interal_data.fncall.func_name = name;
    expr->inner.uni.interal_data.fncall.name_length = strlen(name);
    expr->inner.uni.interal_data.fncall.name_capacity = strlen(name);
}
