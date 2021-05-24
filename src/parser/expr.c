#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "expr.h"
#include "../common.h"

/* ------------------------------------------ */
/*            symbols & values                */
/* ------------------------------------------ */

int symbol_from_token(char *line, Token token, Symbol *value) {
    if (token.token == WORD) {
        // Variable *var = xmalloc(sizeof(Variable));
        value->tag = VariableTag;
        value->inner.variable = xmalloc(token.end - token.start);
        strncpy(value->inner.variable, line + token.start, token.end - token.start);
    }

    else if (token.token == INTEGER || token.token == STRING_LITERAL) {
        //Value *val = xmalloc(sizeof(Value));
        value->tag = ValueTag;
        //value->data_ptr=val;
        
        if (token.token == INTEGER) {
            value->inner.value.type = IntT;
            value->inner.value.data.integer = strtol((line + token.start), NULL, 10);
        }
        else {
            value->inner.value.type = StringT;
            value->inner.value.data.string.length = token.end - token.start;
            value->inner.value.data.string.capacity = value->inner.value.data.string.length + 1; 

            value->inner.value.data.string.ptr = calloc(value->inner.value.data.string.capacity, sizeof(char));
            
            strncpy(
                value->inner.value.data.string.ptr,
                line + token.start,
                value->inner.value.data.string.length
            );
        }
    }

    else 
        return -1;
    
    return 0;
}


int is_func_call(Token tokens[], int nstmt) {
    return tokens[0].token == WORD
    && tokens[1].token == PARAM_OPEN;
}

int is_expr(
    char *line,
    Token tokens[],
    size_t ntokens)
{   
    return (
        tokens[0].token == NOT
        || tokens[0].token == PARAM_OPEN
        || is_data(tokens[0].token)
        || is_func_call(tokens, ntokens)
    );
}

int binop_from_token(enum Lexicon t){
    switch (t) {
        case ADD: return Add;
        case SUB: return Sub;
        case MUL: return Multiply;
        case DIV: return Divide;
        case MOD: return Modolus;
        case POW: return Pow;
        case AND: return And;
        case OR: return Or;
        case ISEQL: return IsEq;
        case ISNEQL: return NotEq;
        case GTEQ: return GtEq;
        case LTEQ: return LtEq;
        case LT: return Lt;
        case GT: return Gt;
        default: return BinaryOperationNop;
    }
}

int construct_expr_inner(
    char *line,
    Token tokens[],
    size_t  ntokens,
    size_t *depth,
    size_t *consumed,
    Expr *expr
){
    size_t last_expr = 0,
           temp = 0;

    // function calls will have multiple expressions inside of them
    // WORD([expression, ..])
    // where each parameter will be evaulated as an expression
    expr->type=UniExprT;
    
    if (tokens[0].token == NOT) {
        *consumed += 1; // for NOT
        struct Expr  
            *rhs = xmalloc(sizeof(struct Expr)),
            *lhs = xmalloc(sizeof(struct Expr));
        
        lhs->type = UniExprT;
        lhs->inner.uni.op = UniValue;
        lhs->inner.uni.interal_data.symbol.tag=ValueTag;
        lhs->inner.uni.interal_data.symbol.inner.value.type=IntT;
        lhs->inner.uni.interal_data.symbol.inner.value.data.integer=0;

        expr->inner.bin.op = IsEq;
        expr->inner.bin.lhs = lhs;
        expr->inner.bin.rhs = rhs;
        expr->type = BinExprT;
        

        if ((temp = construct_expr_inner(line, tokens+1, ntokens, depth, consumed, rhs) == -1))
            return -1;
        //*consumed += temp;
        temp = 0;
    }
    
    else if (tokens[0].token == PARAM_OPEN) {
        // TODO
        // our consume is off target when placed with nested `((expr))` definitions
        *consumed += 1; // open_param
        *depth += 1;
        if ((temp = construct_expr_inner(line, tokens+1, ntokens, depth, consumed, expr)) == -1)
            return -1;
        //*consumed += temp;
        temp = 0;

        *consumed += 1; // closed_param
        last_expr = *consumed-1; // index correction ?
        *depth -= 1;
    }


    else if (is_func_call(tokens, ntokens)) {
        int end_func_flag = 0;
        char *name = malloc(tokens[0].end - tokens[0].start);
        expr->inner.uni.interal_data.fncall.func_name = name;
        memcpy(name, line + tokens[0].start, tokens[0].end - tokens[0].start);

        expr->inner.uni.interal_data.fncall.args = malloc(sizeof(struct Expr *) * 8);
        expr->inner.uni.interal_data.fncall.args_capacity = 7;
        
        last_expr=2;
        while (ntokens > last_expr) {
            if (end_func_flag) break;
            size_t single_expr_idx = 0;
            
            for (size_t i=last_expr; ntokens > i; i++) {
                if (end_func_flag) break;
                // if comma or param_close
                if (tokens[i].token == COMMA) {
                    single_expr_idx=i;
                    break;
                }

                else if (tokens[i].token == PARAM_CLOSE) {
                    end_func_flag = 1;
                    break;
                }
            }
            struct Expr *item = malloc(sizeof(struct Expr));
            expr->inner.uni.interal_data.fncall.args[expr->inner.uni.interal_data.fncall.args_length] = item;
            
            if (construct_expr_inner(line, tokens + last_expr, single_expr_idx, depth, consumed, item) == -1) {
                return -1;
            }

            // TODO
            // check for overflow
            expr->inner.uni.interal_data.fncall.args_length += 1;
            last_expr = single_expr_idx+1;
        }
        expr->inner.uni.op=UniCall;
        expr->type = UniExprT;
    }
        // variable/unit
    else if (is_data(tokens[0].token)) {
        *consumed += 1; // word/value 
        symbol_from_token(line, tokens[0], &expr->inner.uni.interal_data.symbol);
        expr->inner.uni.op=UniValue;
        expr->type=UniExprT;
    }

    else if (tokens[0].token == PARAM_CLOSE || SEMICOLON) {
        return 0;
    }

    else {
        return -1;
    }

    // look ahead for bin operation
    // ============================
    // the reason for the use of `last_expr` here, is so that
    // if do run across a **function**,
    // we try the next token, otherwise
    // it will be 0+1
    if (last_expr != 0 && !is_bin_operator(tokens[last_expr+1].token)) {
        printf("misplaced, points at [%s]\n", ptoken(tokens[last_expr+1].token));
        
        // NOTE: I have no fucking idea why this works.
        // I just noticed a pattern where it would usually be off by
        // the same amount of `depth`
        last_expr -= *depth;
    }
    // --- last_expr should always point at PARAM_CLOSE
    printf("last_expr: %d\n", last_expr);
    if (is_bin_operator(tokens[last_expr+1].token) ) {
            *consumed += 1;
            //struct Expr *parent = xmalloc(sizeof(Expr));
            struct Expr  
                *rhs = xmalloc(sizeof(struct Expr)),
                *lhs = xmalloc(sizeof(struct Expr));
            
            // copy our previously constructed
            // expression into the left hand side (lhs)
            memcpy(lhs, expr, sizeof(struct Expr));
            memset(expr, 0, sizeof(struct Expr)); // for redundency
            
            rhs->depth=*depth;
            lhs->depth=*depth;
            expr->depth=*depth;

            expr->inner.bin.lhs = lhs;
            expr->inner.bin.rhs = rhs;
            expr->inner.bin.op=binop_from_token(tokens[last_expr+1].token);

            if ((temp = construct_expr_inner(line, tokens + last_expr + 2, ntokens, depth, consumed, expr->inner.bin.rhs)) == -1)
                return -1;
            //*consumed += temp;
            
            expr->type=BinExprT;
    }

    return *consumed;
}


int construct_expr(
    char *line,
    Token tokens[],
    size_t  ntokens,
    Expr *expr
){ 
    size_t nconsumed = 0;
    size_t depth = 0;
    if (construct_expr_inner(line, tokens, ntokens, &depth, &nconsumed, expr) != -1)
        return 0;
    return -1;
}

size_t expr_len_inner(Expr *expr, size_t current) {
    if (expr->type==BinExprT) {
        current = expr_len_inner(expr->inner.bin.rhs, current+1);
    }
    return current;
}


size_t expr_len(Expr *expr) {
    return expr_len_inner(expr, 1);
}

