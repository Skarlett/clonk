#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "expr.h"
#include "common.h"

/* ------------------------------------------ */
/*            symbols & values                */
/* ------------------------------------------ */

void init_symbol(struct Symbol *v) {
    v->tag=Literal;
    v->datatype=Null;
    v->data_ptr=0;
}

int symbol_from_token(char *line, struct Token token, struct Symbol *value) {
    init_symbol(value);
    
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
    for (int i=0; FUNC_ARG_SIZE > i; i++)
        init_expression(fn->args[i]);
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

int init_uni_expr_body(UniaryExpr *expr) {
    expr->op=UniaryOperationNop;
    expr->inner=0;
    return 0;
}

int uniary_from_symbol(Symbol *val, UniaryExpr *expr) {
    if (!(val != 0 && expr == 0))
        return -1;

    init_uni_expr_body(expr);
    expr->op=Value;

    Symbol *p = malloc(sizeof(Symbol));
    memcpy(p, val, sizeof(Symbol));

    expr->inner = p;
    
    return 0;
}


/* ------------------------------------------ */
/*             binary expression              */
/* ------------------------------------------ */
// Combinations of uniary expressions


/* ------------------------------------------ */
/*             generic expression struct      */
/* ------------------------------------------ */
// orchestrate symbols/values into expressions...



int is_expr(
    char *line,
    struct Token tokens[],
    size_t ntokens)
{   
    if (is_func_call(tokens, ntokens))
        return TRUE;

    return is_data(tokens[0].token);
}


int construct_expr(
    char *line,
    struct Token tokens[],
    size_t  ntokens,
    struct Expr *expr
){
    size_t last_expr = 0;
    // function calls will have multiple expressions inside of them
    // WORD([expression, ..])
    // where each parameter will be evaulated as an expression

    if (is_func_call(tokens, ntokens)) {
        int end_func_flag = 0;
        FunctionCallExpr *newfunc = malloc(sizeof(struct FunctionCallExpr));  
        init_func_call(newfunc);
        
        while (ntokens > last_expr || end_func_flag) {
            size_t single_expr_idx = 0;
            
            for (size_t i=last_expr; ntokens > i; i++) {
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
            
            Expr *item = newfunc->args[newfunc->args_sz];
            init_expression(item);
            newfunc->args_sz += 1;

            construct_expr(line, tokens + last_expr , single_expr_idx, item);

            last_expr += single_expr_idx;
        }
        
        expr->inner_data = newfunc;
    }

    size_t idx = 0;
    if (last_expr > 0)
        idx = last_expr;
    
    // 
    if (is_bin_operator(tokens[idx+1].token)) {
            BinaryExprBody *body = xmalloc(sizeof(BinaryExprBody));
            body->left_val = xmalloc(sizeof(Expr));
            body->right_val = xmalloc(sizeof(Expr));

            init_expression(body->left_val);
            init_expression(body->left_val);

            enum Lexicon lexed = tokens[idx+1].token;

            switch (lexed) {
                case ADD:
                    body->op=Add;
                    break;
                case SUB:
                    body->op=Sub;
                    break;
                case MUL:
                    body->op=Multiply;
                    break;
                case DIV:
                    body->op=Divide;
                    break;
                case MOD:
                    body->op=Modolus;
                    break;
                case POW:
                    body->op=Pow;
                    break;
                case AND:
                    body->op=And;
                    break;
                case OR:
                    body->op=Or;
                    break;
                default: return -1;
            }

            construct_expr(line, tokens + idx, ntokens, body->left_val);
            construct_expr(line, tokens + idx + 2, ntokens, body->right_val);
    }

    // variable/unit
    else if (is_data(tokens[idx].token)) {
        Symbol unit;
        Expr *n_expr = xmalloc(sizeof(Expr));
        UniaryExpr *uni = xmalloc(sizeof(Expr));
        
        // token into Unit/Symbol
        init_symbol(&unit);
        symbol_from_token(line, tokens[0], &unit);

        // Init an expression
        init_expression(n_expr);
        
        // Init uniary value expression
        init_uni_expr_body(uni);
        
        // set up pointers
        n_expr->inner_data = uni;
        uni=0;

        // finally construct
        uniary_from_symbol(&unit, n_expr->inner_data);
    }

    else {
        return 1;
    }

    return 0;
}


void set_uni_expr(struct Expr *expr, struct UniaryExpr *uni) {
    expr->type=UniExpr;
    expr->inner_data=uni;
}

const char * print_datatype(enum DataType t) {
    switch (t) {
        case String:
            return "string";
        case Int:
            return "integer";
        default: 
            return "undefined";
    }
}


const char * print_bin_operator(enum BinaryOperation t) {
    switch (t) {
        case Add:
            return "addition";
        case Sub:
            return "subtract";
        case Multiply:
            return "multiply";
        case Divide:
            return "divide";
        case Modolus:
            return "modolus";
        case Pow:
            return "power";
        case And:
            return "and";
        case Or:
            return "or";
        default:
            return "unknown";
    }
}

const char * print_symbol_type(enum Tag t) {
    switch (t) {
        case NullTag:
            return "NullTag";
        case Variable:
            return "Variable";
        case Literal:
            return "Literal";
        default: 
            return "undefined";
    }
}

int print_expr(Expr *expr, short unsigned indent){
    tab_print(indent);
    
    if (expr->type == UniExpr) {
        struct UniaryExpr *uni = expr->inner_data;

        if (uni->op == Value) {
                struct Symbol *symbol = uni->inner;
                printf("{\n");
                tab_print(indent+1);
                printf("symbol type: %s\n", print_symbol_type(symbol->tag));
                tab_print(indent+1);
                printf("datatype: %s\n", print_datatype(symbol->datatype));
                tab_print(indent+1);
                printf("}\n");
                tab_print(indent);
        }
        else if (uni->op == Call) {
                struct FunctionCallExpr *fncall = uni->inner;
                printf("{\n");
                tab_print(indent+1);
                printf("function call: %s\n", fncall->func_name);
                tab_print(indent+1);
                printf("parameters: {\n");
                tab_print(indent+2);

                for (int i=0; fncall->args_sz > i; i++){
                    print_expr(fncall->args[i], indent+2);
                    if (fncall->args_sz-1 != i) {
                        printf(",");
                    }
                }
                tab_print(indent+1);
                printf("}\n");
                tab_print(indent);
        }
            
        else {
            printf("\n===============\nERROR \n===============\n");
            return -1;
        }
    }

    else if (expr->type == BinExpr) {
        struct BinaryExprBody *bin = expr->inner_data;
        printf("type: bin-op expr\n");
        tab_print(indent);
        printf("left: {\n");
        tab_print(indent+1);
        print_expr(bin->left_val, indent+1);
        printf("}\n");

        tab_print(indent);
        printf("operator: %s\n", print_bin_operator(bin->op));
        tab_print(indent);

        printf("right: {\n");
        tab_print(indent+1);
        print_expr(bin->left_val, indent+1);
        printf("}\n");
        tab_print(indent);
    }

    else {
        printf("\n===============\nERROR \n===============\n");
        return -1;
    }

    return 0;
}
