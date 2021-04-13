#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "expr.h"

/* ------------------------------------------ */
/*            symbols & values                */
/* ------------------------------------------ */

void init_unit(struct Symbol *v) {
    v->tag=Literal;
    v->datatype=Null;
    v->data_ptr=0;
}

int unit_from_token(char *line, struct Token token, struct Symbol *value) {
    init_unit(value);
    
    if (token.token == WORD) {
        value->tag = Variable;
    }

    else if (token.token == INTEGER) {
        value->tag = Literal;
        value->datatype = Int;
        value->data_ptr = strtol((line + token.start), NULL, 10);
    }

    else if (token.token == STRING_LITERAL) {
        value->tag = Literal;
        value->datatype = String;

        char *inner_data = calloc(STR_STACK_SIZE, sizeof(char));
        for (int i; token.end > i; i++) {
            inner_data[i] = (line + token.start)[i];
        }
        
        value->data_ptr = inner_data;
    }
    else 
        return -1;
    
    return 0;
}


void init_expression(struct Expr *expr) {
    expr->type=UndefinedExpr;
    expr->inner_data=0;
}


void init_func_call(struct FunctionCallExpr *fn) {
    fn->name_sz = 0;
    fn->args_sz = 0;
    for (int i=0; FUNC_ARG_SIZE > i; i++){
        init_expression(&fn->args[i]);
    }
}

int is_func_call(struct Token tokens[], int nstmt) {
    return tokens[0].token == WORD
    && tokens[1].token == PARAM_OPEN
    && tokens[nstmt-2].token == PARAM_CLOSE;
}

/* ------------------------------------------ */
/*             uniary expression              */
/* ------------------------------------------ */
// uniary expressions are either values, or function calls

void init_uni_expr_body(struct UniaryExpr *expr) {
    expr->op=UniaryOperationNop;
    expr->inner=0;
}

int unit_into_uniary(struct Symbol *val, struct UniaryExpr *expr) {
    init_uni_expr_body(expr);    
    expr->op=Value;

    struct Symbol *p = malloc(sizeof(struct Symbol));
    memcpy(p, val, sizeof(struct Symbol));

    expr->inner = p;
    
    return 0;
}


/* ------------------------------------------ */
/*             binary expression              */
/* ------------------------------------------ */
// Combinations of uniary expressions


void init_bin_expr_body(struct BinaryExprBody *expr) {
    expr->op=BinaryOperationNop;
    init_expression(expr->left_val);
    init_expression(expr->right_val);
}
/* ------------------------------------------ */
/*             generic expression struct      */
/* ------------------------------------------ */
// orchestrate symbols/values into expressions...


int construct_expr(
    char *line,
    struct Token tokens[],
    unsigned long  ntokens,
    struct Expr *expr
){

    unsigned long last_expr = 0;

    if (is_func_call(tokens, ntokens)) {
        struct FunctionCallExpr *newfunc = malloc(sizeof(struct FunctionCallExpr));  
        init_func_call(newfunc);
        
        while (ntokens > last_expr) {
            unsigned long single_expr_idx = 0;
            
            for (unsigned long i=last_expr; ntokens > i; i++) {
                // if comma or param_close
                if (tokens[i].token == COMMA) {
                    single_expr_idx=i;
                    break;
                }
            }
            
            struct Expr *item = &newfunc->args[newfunc->args_sz];
            init_expression(item);
            newfunc->args_sz += 1;

            construct_expr(line, tokens + last_expr , single_expr_idx, item);

            last_expr += single_expr_idx;
        }
        
        expr->inner_data = newfunc;
    }

    unsigned long idx = 0;
    if (last_expr > 0)
        idx = last_expr;
    
    if (is_bin_operator(tokens[idx+1].token)) {
            struct BinaryExprBody *body = malloc(sizeof(struct BinaryExprBody));
            init_bin_expr_body(body);

            struct Expr *left = malloc(sizeof(struct Expr));
            struct Expr *right = malloc(sizeof(struct Expr));
            init_expression(left);
            init_expression(right);

            enum Lexicon lexed = tokens[idx+1].token;

            if (lexed == ADD)
                body->op=Add;
            else if (lexed == SUB)
                body->op=Sub;
            else if (lexed == MUL)
                body->op=Multiply;
            else if (lexed == DIV)
                body->op=Divide;
            else if (lexed == MOD)
                body->op=Modolus;
            else if (lexed == AND)
                body->op=And;
            else if (lexed == OR)
                body->op=Or;
            else if (lexed == POW)
                body->op=Pow;
            else return -1;

            construct_expr(line, tokens, 0, left);
            body->left_val = left;

            construct_expr(line, tokens + 2, ntokens, right);
            body->left_val = right;
    }

    // variable/unit
    else if (is_data(tokens[idx].token)) {
        
        struct Symbol unit;
        init_unit(&unit);
        unit_from_token(line, tokens[0], &unit);
        
        struct Expr *n_expr = malloc(sizeof(struct Expr));
        init_expression(n_expr);
        
        unit_into_uniary(&unit, n_expr->inner_data);
    }
}


void set_uni_expr(struct Expr *expr, struct UniaryExpr *uni) {
    expr->type=UniExpr;
    expr->inner_data=uni;
}



