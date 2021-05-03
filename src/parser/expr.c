#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "expr.h"
#include "../common.h"

/* ------------------------------------------ */
/*            symbols & values                */
/* ------------------------------------------ */

void init_symbol(Symbol *v) {
    v->tag=NullTag;
    v->data_ptr=0;
}

int symbol_from_token(char *line, Token token, Symbol *value) {
    if (token.token == WORD) {
        Variable *var = xmalloc(sizeof(Variable));
        value->tag = VariableTag;
        value->data_ptr = var;
        strncpy(var->word, line + token.start, token.end - token.start);
    }

    else if (
        token.token == INTEGER
        || token.token == STRING_LITERAL
    ) {
        Value *val = xmalloc(sizeof(Value));
        value->tag = ValueTag;
        value->data_ptr=val;
        
        if (token.token == INTEGER) {
            val->datatype = IntT;
            val->data_ptr = strtoll((line + token.start), NULL, 10);
        }
        else {
            val->datatype = StringT;
            val->data_ptr = calloc(token.end - token.start + 1, sizeof(char));
            strncpy(val->data_ptr, line + token.start, token.end - token.start);
        }
    }

    else 
        return -1;
    
    return 0;
}


void init_expression(Expr *expr) {
    expr->type=UndefinedExprT;
    expr->inner_data=0;
}

void init_func_call(FunctionCallExpr *fn) {
    fn->name_sz = 0;
    fn->args_sz = 0;
    for (int i=0; FUNC_ARG_SIZE > i; i++)
        init_expression(fn->args[i]);
}

int is_func_call(Token tokens[], int nstmt) {
    return tokens[0].token == WORD
    && tokens[1].token == PARAM_OPEN
    && tokens[nstmt-2].token == PARAM_CLOSE;
}

int is_expr(
    char *line,
    Token tokens[],
    size_t ntokens)
{   
    if (is_func_call(tokens, ntokens))
        return TRUE;

    return is_data(tokens[0].token);
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
        case GTEQ: return GtEq;
        case LTEQ: return LtEq;
        case LT: return Lt;
        case GT: return Gt;
        default: return BinaryOperationNop;
    }
}

int construct_expr(
    char *line,
    Token tokens[],
    size_t  ntokens,
    Expr *expr
){
    size_t last_expr = 0;
    // function calls will have multiple expressions inside of them
    // WORD([expression, ..])
    // where each parameter will be evaulated as an expression
    expr->type=UniExprT;
    
    if (is_func_call(tokens, ntokens)) {
        UniExpr *uni = xmalloc(sizeof(UniExpr));
        FunctionCallExpr *fncall = xmalloc(sizeof(FunctionCallExpr));

        int end_func_flag = 0;
        init_func_call(fncall);
        
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
            
            Expr *item = fncall->args[fncall->args_sz];
            init_expression(item);
            construct_expr(line, tokens + last_expr, single_expr_idx, item);

            fncall->args_sz += 1;
            last_expr += single_expr_idx;
        }
        uni->op=Call;
        uni->inner = fncall;
        expr->inner_data = uni;
        expr->type = UniExprT;
    }

    // variable/unit
    else if (is_data(tokens[0].token)) {
        Symbol *unit = xmalloc(sizeof(Symbol));
        UniExpr *uni = xmalloc(sizeof(UniExpr));
        uni->inner=0;
        
        // token into Unit/Symbol
        init_symbol(unit);
        symbol_from_token(line, tokens[0], unit);
        
        uni->inner = unit;
        uni->op=UniValue;
        expr->type=UniExprT;
        expr->inner_data=uni;
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
    if (is_bin_operator(tokens[last_expr+1].token)) {
            BinExpr *body = xmalloc(sizeof(BinExpr));
            Expr rhs;
            init_expression(&rhs);

            body->left_val = *expr;
            body->right_val=rhs;

            body->op=binop_from_token(tokens[last_expr+1].token);

            if (construct_expr(line, tokens + last_expr + 2, ntokens, &body->right_val) != 0)
                return -1;
            
            expr->inner_data=body;
            expr->type=BinExprT;
    }

    return 0;
}


void set_uni_expr(struct Expr *expr, struct UniExpr *uni) {
    expr->type=UniExprT;
    expr->inner_data=uni;
}

const char * print_datatype(enum DataType t) {
    switch (t) {
        case StringT:
            return "string";
        case IntT:
            return "integer";
        case NullT:
            return "null";
        default: 
            return "undefined";
    }
}
const char * print_expr_t(enum ExprType t) {
    switch (t) {
        case UniExprT:
            return "uniary";
        case BinExprT:
            return "binary";
        case UndefinedExprT:
            return "null";
        default: 
            return "undefined";
    }
}


const char * print_bin_operator(enum BinOp t) {
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
        case GtEq:
            return "gteq";
        case Gt:
            return "gt";
        case Lt:
            return "lt";
        case LtEq:
            return "lteq";
        case IsEq:
            return "iseq";
        default:
            return "unknown";
    }
}

const char * print_symbol_type(enum Tag t) {
    switch (t) {
        case NullTag:
            return "NullTag";
        case VariableTag:
            return "Variable";
        case ValueTag:
            return "Literal";
        default: 
            return "undefined";
    }
}

int print_expr(Expr *expr, short unsigned indent){
    tab_print(indent);
    printf("type: %s\n", print_expr_t(expr->type));

    if (expr->type == UniExprT) {
        struct UniExpr *uni = expr->inner_data;

        if (uni->op == UniValue) {
                struct Symbol *symbol = uni->inner;
                tab_print(indent);
                printf("symbol type: %s\n", print_symbol_type(symbol->tag));
                tab_print(indent);
                if (symbol->tag == VariableTag) {
                    printf("symbol name: %s\n", (char *)symbol->data_ptr);
                }
                else if (symbol->tag == ValueTag) {
                    Value *val = symbol->data_ptr;
                    printf("symbol val: ");
                    if (val->datatype == IntT)
                        printf("%lld\n", (long long)val->data_ptr);
                    else if (val->datatype == StringT)
                        printf("\"%s\"\n", (char *)val->data_ptr);
                    else if (val->datatype == NullT) {
                        printf("null\n");
                    }
                    else {
                        printf("undefined\n");
                    }
                }
                else {
                    printf("error: got unexpected tag");
                }
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

    else if (expr->type == BinExprT) {
        struct BinExpr *bin = expr->inner_data;

        tab_print(indent);
        printf("operator: %s\n", print_bin_operator(bin->op));
        tab_print(indent);

        
        printf("left: {\n");
        print_expr(&bin->left_val, indent+1);
        tab_print(indent);
        printf("},\n");

        tab_print(indent);
        printf("right: {\n");
        print_expr(&bin->right_val, indent+1);
        tab_print(indent);
        printf("}\n");
    }

    else {
        printf("\n===============\nERROR \n===============\n");
        return -1;
    }

    return 0;
}