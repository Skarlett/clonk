#ifndef _HEADER__EXPR_HDLR__
#define _HEADER__EXPR_HDLR__

#include "../lexer/lexer.h"
#include "expr.h"

void mk_null(struct Expr *ex);

int8_t mk_str(struct ExprParserState *state, struct Expr *ex); 
int8_t mk_int(struct ExprParserState *state, struct Expr *ex);
int8_t mk_symbol(struct ExprParserState *state, struct Expr *ex);

int8_t mk_operator(struct ExprParserState *state, struct Expr *ex, struct Token *op_head);
int8_t mk_group(struct ExprParserState *state, struct Expr *ex);

int8_t mk_binop(struct Token *op, struct ExprParserState *state, struct Expr *ex);
int8_t mk_not(struct ExprParserState *state, struct Expr *ex);
int8_t mk_idx_access(struct ExprParserState *state, struct Expr *ex);
int8_t mk_fncall(struct ExprParserState *state, struct Expr *ex);

enum Operation operation_from_token(enum Lexicon token);
void determine_return_ty(struct Expr *bin);

int8_t mk_if_cond(struct ExprParserState *state, struct Expr *ex);
int8_t mk_if_body(struct ExprParserState *state);
int8_t mk_else_body(struct ExprParserState *state);

int8_t mk_return(struct ExprParserState *state, struct Expr *ex);
int8_t mk_def_sig(struct ExprParserState *state, struct Expr *ex);
int8_t mk_def_body(struct ExprParserState *state);

#endif

