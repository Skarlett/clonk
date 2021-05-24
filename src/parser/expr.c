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
    && tokens[1].token == PARAM_OPEN
    && tokens[nstmt-2].token == PARAM_CLOSE;
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
        //last_expr += 2;
        char *name = malloc(tokens[0].end - tokens[0].start);
        expr->inner.uni.interal_data.fncall.func_name = name;
        memcpy(name, line + tokens[0].start, tokens[0].end - tokens[0].start);

        expr->inner.uni.interal_data.fncall.args = malloc(sizeof(struct Expr *) * 8);
        expr->inner.uni.interal_data.fncall.args_capacity = 7;

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
            
            if (construct_expr_inner(line, tokens + 2 + last_expr, single_expr_idx, depth, consumed, item) == -1) {
                return -1;
            }

            // TODO
            // check for overflow
            expr->inner.uni.interal_data.fncall.args_length += 1;
            last_expr += single_expr_idx;
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


const char * print_datatype(enum DataType t) {
    switch (t) {
        case StringT: return "string";
        case IntT: return "integer";
        case NullT: return "null";
        default: return "undefined";
    }
}
const char * print_expr_t(enum ExprType t) {
    switch (t) {
        case UniExprT: return "unitary";
        case BinExprT: return "binary";
        case UndefinedExprT: return "null";
        default: return "undefined";
    }
}


const char * print_bin_operator(enum BinOp t) {
    switch (t) {
        case Add: return "addition";
        case Sub: return "subtract";
        case Multiply: return "multiply";
        case Divide: return "divide";
        case Modolus: return "modolus";
        case Pow: return "power";
        case And: return "and";
        case Or: return "or";
        case GtEq: return "gteq";
        case Gt: return "gt";
        case Lt: return "lt";
        case LtEq: return "lteq";
        case IsEq: return "iseq";
        case NotEq: return "noteq";
        default: return "unknown";
    }
}
const char * p_bin_operator_sym(enum BinOp t) {
    switch (t) {
        case Add: return "+";
        case Sub: return "-";
        case Multiply: return "*";
        case Divide: return "/";
        case Modolus: return "\%";
        case Pow: return "^";
        case And: return "&&";
        case Or: return "||";
        case GtEq: return "<=";
        case Gt: return "<";
        case Lt: return ">";
        case LtEq: return ">=";
        case IsEq: return "==";
        case NotEq: return "!=";
        default: return "unknown";
    }
}

const char * print_symbol_type(enum Tag t) {
    switch (t) {
        case NullTag: return "NullTag";
        case VariableTag: return "Variable";
        case ValueTag: return "Literal";
        default: return "undefined";
    }
}

int print_expr(Expr *expr, short unsigned indent){
    tab_print(indent);
    printf("type: %s\n", print_expr_t(expr->type));
    tab_print(indent);
    printf("depth: %d\n", expr->depth);

    if (expr->type == UniExprT) {
        //struct UniExpr *uni = expr->inner_data;
        if (expr->inner.uni.op == UniValue) {
            tab_print(indent);
                
            printf("symbol type: %s\n", print_symbol_type(expr->inner.uni.interal_data.symbol.tag));
            tab_print(indent);
            printf("symbol val: ");
                
            switch (expr->inner.uni.interal_data.symbol.tag) {
                case VariableTag:
                    printf("symbol name: %s\n", expr->inner.uni.interal_data.symbol.inner.variable);
                    break;
                    
                case ValueTag:
                    switch (expr->inner.uni.interal_data.symbol.inner.value.type) {
                        case IntT:
                            printf("%d\n", expr->inner.uni.interal_data.symbol.inner.value.data.integer);
                            break;
                            
                        case StringT:
                            printf("%s\n", expr->inner.uni.interal_data.symbol.inner.value.data.string.ptr);
                            break;
                            
                        case NullT:
                            printf("null");
                            break;
                            
                        default: 
                            printf("error: got unexpected datatype");
                            break;
                        }

                        break;
                    
                    default:
                        printf("error: got unexpected tag");
                        break;
                }
        }
        

        else if (expr->inner.uni.op == UniCall) {
                printf("{\n");
                tab_print(indent+1);
                printf("function call: %s\n", expr->inner.uni.interal_data.fncall.func_name);
                tab_print(indent+1);
                printf("parameters: {\n");
                tab_print(indent+2);
                
                for (int i=0; expr->inner.uni.interal_data.fncall.args_length > i; i++){
                    print_expr(expr->inner.uni.interal_data.fncall.args[i], indent+2);
                    if (expr->inner.uni.interal_data.fncall.args_length-1 != i) {
                        printf(",");
                    }
                }
                tab_print(indent+1);
                printf("}\n");
                tab_print(indent);
            
        }
        
    }

    else if (expr->type == BinExprT) {
        //struct BinExpr *bin = ;

        tab_print(indent);
        printf("operator: %s (%s)\n", p_bin_operator_sym(expr->inner.bin.op), print_bin_operator(expr->inner.bin.op));
        tab_print(indent);

        
        printf("left: {\n");
        if (expr != expr->inner.bin.lhs) 
            print_expr(expr->inner.bin.lhs, indent+1);
        
        tab_print(indent);
        printf("},\n");

        tab_print(indent);
        printf("right: {\n");
        print_expr(expr->inner.bin.rhs, indent+1);
        tab_print(indent);
        printf("}\n");
    }

    else {
        printf("\n===============\nERROR \n===============\n");
        return -1;
    }

    return 0;
}

#define tb "├"
#define rb "─"
#define fb "└"

void small_tab(short unsigned indent) {
    for (int i = 0; indent > i; i++) 
        printf("  ");
}

void ptree_inner(Expr *expr, short unsigned indent){
    if (expr->type == BinExprT) {
        printf("%s ", p_bin_operator_sym(expr->inner.bin.op));
        tab_print(5-indent);
        printf("\t(%d) \n", expr->depth);
        small_tab(indent);
        printf("├─");
        ptree_inner(expr->inner.bin.lhs, indent+1);
        printf("\n");
        small_tab(indent);
        printf("└─");
        ptree_inner(expr->inner.bin.rhs, indent+1);
        
    }
    else if (expr->type == UniExprT) {
        if (expr->inner.uni.op == UniValue) {
            if (expr->inner.uni.interal_data.symbol.tag == ValueTag) {
                if (expr->inner.uni.interal_data.symbol.inner.value.type == IntT) {
                    printf("%d", expr->inner.uni.interal_data.symbol.inner.value.data.integer);
                }
                else if (expr->inner.uni.interal_data.symbol.inner.value.type == StringT) {
                    printf("STR");
                }
            }
            else if (expr->inner.uni.interal_data.symbol.tag == VariableTag) {
                printf("%s", expr->inner.uni.interal_data.symbol.inner.variable);
            }
        }

        else if (expr->inner.uni.op == UniCall) {
            printf("FCALLS UNSUPPORTED");
        }
    }
}


void ptree(Expr *expr) {
    ptree_inner(expr, 0);
    printf("\n");
}