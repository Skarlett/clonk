#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../CuTest.h"
#include "../common.h"

#include "lexer.h"
#include "expr.h"
#include "expr_debug.h"

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
           ret = 0,
           tmp = 0;
    
    expr->type=UniExprT;
    expr->depth=*depth;

    if (tokens[0].token == NOT) {
        ret += 1; // for NOT
        *consumed += 1;
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
        
        if ((tmp = construct_expr_inner(line, tokens+1, ntokens, depth, consumed, rhs) == -1))
            return -1;
        ret += tmp;
        tmp = 0;
    }

    else if (tokens[0].token == PARAM_CLOSE || SEMICOLON) {
        return 1;
    }
    
    else if (tokens[0].token == PARAM_OPEN) {
        ret += 1;
        // TODO
        // our consume is off target when placed with nested `((expr))` definitions
        //*consumed += 2; // open_param + closed_param
        *depth += 1;
        if ((tmp = construct_expr_inner(line, tokens+1, ntokens, depth, consumed, expr)) == -1)
            return -1;
        *depth -= 1;
        ret += tmp + 1;
        tmp = 0;
        
        last_expr = ret - 1; // index correction ?
    }

    else if (is_func_call(tokens, ntokens)) {
        int end_func_flag = 0;
        char *name = malloc(tokens[0].end - tokens[0].start);

        expr->inner.uni.interal_data.fncall.func_name = name;
        memcpy(name, line + tokens[0].start, tokens[0].end - tokens[0].start);

        if (tokens[2].token != PARAM_CLOSE) {
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
        }
        expr->inner.uni.op=UniCall;
        expr->type = UniExprT;
        expr->depth=*depth;
    }
    
    // variable/unit
    else if (is_data(tokens[0].token)) {
        *consumed += 1;
        ret += 1; // word/value 
        symbol_from_token(line, tokens[0], &expr->inner.uni.interal_data.symbol);
        expr->inner.uni.op=UniValue;
        expr->type=UniExprT;
        expr->depth=*depth;
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

    // --- last_expr should always point at PARAM_CLOSE
    if (last_expr != 0) {     
        if (tokens[last_expr].token != PARAM_CLOSE && tokens[last_expr].token != SEMICOLON) {
            printf("misplaced [%s] [%d]\n", ptoken(tokens[last_expr].token), last_expr);
        }
        if (!is_bin_operator(tokens[last_expr+1].token))
            printf("expects binary op - got [%s] [%d]\n", ptoken(tokens[last_expr+1].token), last_expr);
    }
#ifdef DEBUG
    printf("last_expr: %d\n", last_expr);
#endif

    if (is_bin_operator(tokens[last_expr+1].token) ) {
            ret += 1;
            //struct Expr *parent = xmalloc(sizeof(Expr));
            struct Expr  
                *rhs = xmalloc(sizeof(struct Expr)),
                *lhs = xmalloc(sizeof(struct Expr));
            
            // copy our previously constructed
            // expression into the left hand side (lhs)
            memcpy(lhs, expr, sizeof(struct Expr));
            memset(expr, 0, sizeof(struct Expr)); // for redundency
            
            expr->inner.bin.lhs = lhs;
            expr->inner.bin.rhs = rhs;
            expr->inner.bin.op = binop_from_token(tokens[last_expr+1].token);

            if ((tmp = construct_expr_inner(line, tokens + last_expr + 2, ntokens, depth, consumed, expr->inner.bin.rhs)) == -1)
                return -1;
            
            ret += tmp;
            
            expr->type=BinExprT;
    }

    return ret;
}

int construct_expr(
    char *line,
    Token tokens[],
    usize  ntokens,
    Expr *expr
){ 
    usize nconsumed = 0;
    usize depth = 0;

    memset(expr, 0, sizeof(struct Expr));
    
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


int cmpexpr(struct Expr *a, struct  Expr *b) {
    // pattern matching would be awesome right now.

    if (a->type != b->type || a->depth != b->depth) {
        return 0;
    }

    else if (a->type == UndefinedExprT) {
        return 1;
    }
    
    else if (a->type == BinExprT) {
        return cmpexpr(a->inner.bin.lhs, b->inner.bin.lhs) == 1 && 
        cmpexpr(a->inner.bin.rhs, b->inner.bin.rhs) == 1;
    }
    
    else if (a->type == UniExprT) {
        if (a->inner.uni.op != b->inner.uni.op) { return 0; }

        else if (a->inner.uni.op == UniaryOperationNop) { return 1; }

        if (a->inner.uni.op == UniCall) {
            if (a->inner.uni.interal_data.fncall.name_length == b->inner.uni.interal_data.fncall.name_length
                && a->inner.uni.interal_data.fncall.args_length == b->inner.uni.interal_data.fncall.args_length) {
                int ret;
                
                if (strncmp(
                    a->inner.uni.interal_data.fncall.func_name,
                    b->inner.uni.interal_data.fncall.func_name,
                    a->inner.uni.interal_data.fncall.name_length) == 0)
                {
                    for (usize i=0; a->inner.uni.interal_data.fncall.args_length > i; i++) {
                        if (ret = cmpexpr(a->inner.uni.interal_data.fncall.args[i], b->inner.uni.interal_data.fncall.args[i]) != 1) {
                            return ret;                            
                        };
                    }
                    return 1;
                }
            }
            else {return 0;}
        }

        // Compare Value and Variables
        else if (a->inner.uni.op == UniValue) {
            
            // Compare symbol tags, and then determine if the expression is a variable or a value
            if (a->inner.uni.interal_data.symbol.tag == b->inner.uni.interal_data.symbol.tag)
            {
                // This is a Value
                if (a->inner.uni.interal_data.symbol.tag == ValueTag) {
                    
                    // Check Values have same type
                    if (a->inner.uni.interal_data.symbol.inner.value.type !=
                        b->inner.uni.interal_data.symbol.inner.value.type) { return 0; }

                    // Compare literal values.
                    switch (a->inner.uni.interal_data.symbol.inner.value.type) {
                        case IntT:
                            return a->inner.uni.interal_data.symbol.inner.value.data.integer == b->inner.uni.interal_data.symbol.inner.value.data.integer; 
                        
                        case StringT:
                            if (a->inner.uni.interal_data.symbol.inner.value.data.string.length != b->inner.uni.interal_data.symbol.inner.value.data.string.length) {
                                return 0;
                            }
                            return strncmp(
                                a->inner.uni.interal_data.symbol.inner.value.data.string.ptr,
                                b->inner.uni.interal_data.symbol.inner.value.data.string.ptr,
                                a->inner.uni.interal_data.symbol.inner.value.data.string.length
                            ) == 0;
                        
                        // Null Datatype
                        case NullT:
                            return 1;

                        // Should be unreachable   
                        default:
                            return -1;
                    }
                }

                // This is a variable
                else if (a->inner.uni.interal_data.symbol.tag == VariableTag) {
                    return strcmp(
                        a->inner.uni.interal_data.symbol.inner.variable,
                        b->inner.uni.interal_data.symbol.inner.variable
                    ) == 0;
                }

                // NullTag 
                else { return 1; }
            }
            // a...symbol.tag == b...symbol.tag
            else { return 0; }
        }
    }
}

size_t expr_len(Expr *expr) {
    return expr_len_inner(expr, 1);
}

#ifdef INCLUDE_TESTS

void build_bin_expr_T(Expr *expr, Expr *lhs, Expr *rhs, enum BinOp op) {
    expr->type = BinExprT;
    expr->inner.bin.op = op;
    expr->inner.bin.lhs = lhs;
    expr->inner.bin.rhs = rhs;
}

void build_int_value_T(Expr *expr, int value) {
    expr->type = UniExprT;
    expr->inner.uni.op = UniValue;
    expr->depth = 0;
    expr->inner.uni.interal_data.symbol.tag = ValueTag;
    expr->inner.uni.interal_data.symbol.inner.value.type = IntT;
    expr->inner.uni.interal_data.symbol.inner.value.data.integer = value;
}

void build_str_value_T(Expr *expr, char * str) {
    expr->type = UniExprT;
    expr->inner.uni.op = UniValue;
    expr->depth = 0;
    expr->inner.uni.interal_data.symbol.tag = ValueTag;
    expr->inner.uni.interal_data.symbol.inner.value.type = StringT;
    expr->inner.uni.interal_data.symbol.inner.value.data.string.ptr = malloc(strlen(str)+1);
    expr->inner.uni.interal_data.symbol.inner.value.data.string.length = strlen(str);
    expr->inner.uni.interal_data.symbol.inner.value.data.string.capacity = strlen(str)+1;
}

void build_var_T(Expr *expr, char * var_name) {
    expr->type = UniExprT;
    expr->inner.uni.op = UniValue;
    expr->depth = 0;
    expr->inner.uni.interal_data.symbol.tag = VariableTag;
    expr->inner.uni.interal_data.symbol.inner.variable = malloc(strlen(var_name));
}

void build_fn_call_T(Expr *expr, char *name, Expr *args[], u16 argc) {
    expr->type = UniExprT;
    expr->inner.uni.op = UniCall;
    expr->depth = 0;
    expr->inner.uni.interal_data.fncall.args = args;
    expr->inner.uni.interal_data.fncall.args_capacity = argc;
    expr->inner.uni.interal_data.fncall.args_length = argc;
    expr->inner.uni.interal_data.fncall.func_name = name;
    expr->inner.uni.interal_data.fncall.name_length = strlen(name);
    expr->inner.uni.interal_data.fncall.name_capacity = strlen(name);
}


int __check_tokens(struct Token tokens[], enum Lexicon *lexicon, usize len){
    for (int i=0; len > i; i++) {
        if (tokens[i].token != lexicon[i]) {
            return -1;
        }
    }
    return 0;
}
/*
  Prove incremential changes cause comparsion failure.
*/
void __test__sanity_expr_cmp(CuTest* tc)
{
    struct Expr a, b;
    
    a.type = UniExprT;
    a.inner.uni.op = UniValue;
    a.depth = 0;

    a.inner.uni.interal_data.symbol.tag = ValueTag;
    a.inner.uni.interal_data.symbol.inner.value.type = IntT;
    a.inner.uni.interal_data.symbol.inner.value.data.integer = 1;

    CuAssertTrue(tc, cmpexpr(&a, &a) == 1);

    b.type = UniExprT;
    b.inner.uni.op = UniValue;
    b.inner.uni.interal_data.symbol.tag = VariableTag;
    b.inner.uni.interal_data.symbol.inner.variable = "a";
    
    CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
    CuAssertTrue(tc, cmpexpr(&b, &b) == 1);

    a.inner.uni.interal_data.symbol.tag = VariableTag;
    a.inner.uni.interal_data.symbol.inner.variable = "b";

    a.type = UniExprT;
    a.inner.uni.op = UniValue;
    a.depth = 0;

    b.type = UniExprT;
    b.inner.uni.op = UniValue;
    b.depth = 0;

    CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
    a.inner.uni.interal_data.symbol.inner.variable = "a";
    CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
    a.type = UndefinedExprT;
    CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
    b.type = UndefinedExprT;
    CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
    a.type = UniExprT;
    b.type = UniExprT;


    a.inner.uni.op = UniaryOperationNop;
    CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
    
    a.inner.uni.op = UniValue;
    CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

    a.inner.uni.interal_data.symbol.tag = NullTag;
    a.type = UniExprT;
    a.inner.uni.op = UniValue;
    a.depth = 0;

    CuAssertTrue(tc, cmpexpr(&a, &b) == 0);

    b.inner.uni.interal_data.symbol.tag = NullTag;
    CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

    b.type = UniExprT;
    b.inner.uni.op = UniValue;
    b.depth = 0;

    a.inner.uni.interal_data.symbol.tag = ValueTag;
    b.inner.uni.interal_data.symbol.tag = ValueTag;

    a.inner.uni.interal_data.symbol.inner.value.type = IntT;
    b.inner.uni.interal_data.symbol.inner.value.type = IntT;
    a.inner.uni.interal_data.symbol.inner.value.data.integer = 1;
    b.inner.uni.interal_data.symbol.inner.value.data.integer = 2;

    CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
    b.inner.uni.interal_data.symbol.inner.value.data.integer = 1;
    CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

    b.type = UniExprT;
    b.inner.uni.op = UniValue;
    b.depth = 0;

    b.inner.uni.interal_data.symbol.inner.value.type = StringT;
    b.inner.uni.interal_data.symbol.inner.value.data.string.capacity = 0;
    b.inner.uni.interal_data.symbol.inner.value.data.string.length = 5;
    b.inner.uni.interal_data.symbol.inner.value.data.string.ptr = "data\0";
    
    CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
    a.type = UniExprT;
    a.inner.uni.op = UniValue;
    a.depth = 0;

    a.inner.uni.interal_data.symbol.inner.value.type = StringT;
    a.inner.uni.interal_data.symbol.inner.value.data.string.capacity = 0;
    a.inner.uni.interal_data.symbol.inner.value.data.string.length = 5;
    a.inner.uni.interal_data.symbol.inner.value.data.string.ptr = "data\0";
    CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
    
    struct Expr bin;
    bin.type = BinExprT;
    bin.inner.bin.lhs = &a;
    bin.inner.bin.rhs = &b;

    CuAssertTrue(tc, cmpexpr(&bin, &bin) == 1);
    CuAssertTrue(tc, cmpexpr(&a, &bin) == 0);

    //TODO add test cases for fncalls
}


void __test__double_perthensis_1(CuTest* tc) {
    static char * line = "((1))";
    struct Token tokens[16];
    struct Expr expr[2];
    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=2;

    ntokens = tokenize(line, tokens, 0);
    
    CuAssertTrue(tc, ntokens == 1);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[1]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[0], &expr[1]) == 1);
}

void __test__double_perthensis_2(CuTest* tc) {
    static char * line = "((1)) + 2";
    struct Token tokens[16];
    struct Expr expr[8];
    usize ntokens;
    
    expr[0].depth=2;
    build_int_value_T(&expr[0], 1);
    build_int_value_T(&expr[1], 2);
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 7);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}

void __test__double_perthensis_3(CuTest* tc) {
    static char * line = "((1)) + ((2))";
    struct Token tokens[16];
    struct Expr expr[8];

    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=2;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=2;
    
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}

void __test__double_perthensis_4(CuTest* tc) {
    static char * line = "(1 + (2))";
    struct Token tokens[16];
    struct Expr expr[8];
    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=1;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=2;
    
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
    ntokens = tokenize(line, tokens, 0);
    CuAssertTrue(tc, ntokens == 7);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}


void __test__double_perthensis_5(CuTest* tc) {
    static char * line = "((1) + 2)";
    struct Token tokens[16];
    struct Expr expr[8];
    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=2;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=1;
    
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
    ntokens = tokenize(line, tokens, 0);
    CuAssertTrue(tc, ntokens == 7);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}

void __test__double_perthensis_6(CuTest* tc) {
    static char * line = "((1) + (2))";
    struct Token tokens[16];
    struct Expr expr[8];
    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=2;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=2;
    
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
    ntokens = tokenize(line, tokens, 0);
    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}


void __test__double_perthensis_7(CuTest* tc) {
    static char * line = "((1 + 2))";
    struct Token tokens[16];
    struct Expr expr[8];

    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=2;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=2;
    
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 7);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}

void __test__double_perthensis_8(CuTest* tc) {
    static char * line = "((1) + 2) + 3";
    struct Token tokens[16];
    struct Expr expr[8];

    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=2;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=1;

    build_int_value_T(&expr[2], 3);

    build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    expr[2].depth=1;
    
    build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);
    
    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);
}


void __test__double_perthensis_9(CuTest* tc) {
    static char * line = "1 + (2 + (3))";
    struct Token tokens[16];
    struct Expr expr[8];

    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=2;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=1;

    build_int_value_T(&expr[2], 3);

    build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    expr[3].depth=1;

    build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);
    expr[4].depth=2;

    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);
}

void __test__double_perthensis_10(CuTest* tc) {
    static char * line = "1 + ((2) + 3)";
    struct Token tokens[16];
    struct Expr expr[8];

    usize ntokens;
    
    build_int_value_T(&expr[0], 1);

    build_int_value_T(&expr[1], 2);
    expr[1].depth=2;
    
    build_int_value_T(&expr[2], 3);
    expr[2].depth = 1;

    build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    expr[3].depth=0;
    
    build_bin_expr_T(&expr[4], &expr[2], &expr[3], Add);
    
    expr[4].depth=1;
    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[3], &expr[5]) == 1);
}

void __test__double_perthensis_11(CuTest* tc) {
    static char * line = "((1 + 2 + 3))";
    struct Token tokens[16];
    struct Expr expr[8];

    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    build_int_value_T(&expr[1], 2);
    build_int_value_T(&expr[2], 3);

    expr[2].depth=1;
    expr[1].depth=2;
    
    build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    expr[3].depth=0;
    
    build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);
    expr[4].depth=1;
    
    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[3], &expr[5]) == 1);
}

void __test__double_perthensis_12(CuTest* tc) {
    static char * line = "(1 + (2) + 3)";
    struct Token tokens[16];
    struct Expr expr[8];
    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=1;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=2;

    build_int_value_T(&expr[2], 2);
    expr[2].depth=1;
    
    build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    expr[3].depth=1;
    
    build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);

    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);
}


void __test__double_perthensis_13(CuTest* tc) {
    static char * line = "1 + ((2 + 3))";
    struct Token tokens[16];
    struct Expr expr[8];
    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    build_int_value_T(&expr[1], 2);
    build_int_value_T(&expr[2], 3);
    
    expr[2].depth=2;
    expr[1].depth=2;
    
    build_bin_expr_T(&expr[3], &expr[1], &expr[2], Add);
    build_bin_expr_T(&expr[4], &expr[0], &expr[3], Add);
    
    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[3], &expr[5]) == 1);
}


void __test__double_perthensis_14(CuTest* tc) {
    static char * line = "((1 + 2)) + 3";
    struct Token tokens[16];
    struct Expr expr[8];
    usize ntokens;
    
    for (int i=0; 3 > i; i++) {
        build_int_value_T(&expr[i], i+1);
        expr[i].depth=2;
    }

    expr[2].depth = 0;

    build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);

    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);
}

void __test__double_perthensis_15(CuTest* tc) {
    static char * line = "((1 + 2)) + (( 3 + 4))";
    struct Token tokens[16];
    struct Expr expr[8];
    usize ntokens;

    for (int i=0; 4 > i; i++) {
        build_int_value_T(&expr[i], i+1);
        expr[i].depth=2;
    }

    build_bin_expr_T(&expr[5], &expr[0], &expr[1], Add);
    expr[5].depth=2;

    build_bin_expr_T(&expr[6], &expr[2], &expr[3], Add);
    expr[6].depth=2;
    
    build_bin_expr_T(&expr[7], &expr[5], &expr[6], Add);    
    expr[7].depth=0;
    
    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[8]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[7], &expr[8]) == 1);
}


void __test__double_perthensis_16(CuTest* tc) {
    static char * line = "((1 + 2) + (3 + 4))";
    struct Token tokens[16];
    struct Expr expr[8];

    usize ntokens;

    for (int i=0; 4 > i; i++) {
        build_int_value_T(&expr[i], i+1);
        expr[i].depth=2;
    }

    build_bin_expr_T(&expr[5], &expr[0], &expr[1], Add);
    expr[5].depth=2;

    build_bin_expr_T(&expr[6], &expr[2], &expr[3], Add);
    expr[6].depth=2;

    build_bin_expr_T(&expr[7], &expr[5], &expr[6], Add);
    expr[7].depth=1;

    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[8]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[7], &expr[8]) == 1);
}


void __test__double_perthensis_17(CuTest* tc) {
    static char * line = "(( ((1 + 2)) + (3 + 4) ))";
    struct Token tokens[16];
    struct Expr expr[8];

    usize ntokens;
    
    build_int_value_T(&expr[0], 1);

    build_int_value_T(&expr[1], 2);
    expr[1].depth=2;
    
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    expr[2].depth=0;
    
    expr[3].depth=1;
    expr[3].type = BinExprT;
    expr[3].inner.bin.op = Add;
    expr[3].inner.bin.lhs = &expr[2];
    
    build_int_value_T(&expr[4], 3);
    expr[3].inner.bin.rhs = &expr[4];
    expr[4].depth = 1;

    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 9);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[3], &expr[5]) == 1);
}


void __test__sanity_test_1(CuTest* tc) {
    static char * line = "1";
    usize ntokens;
    struct Token tokens[16];
    struct Expr expr[2];
    
    build_int_value_T(&expr[0], 1);

    ntokens = tokenize(line, tokens, 0);
    
    CuAssertTrue(tc, ntokens == 1);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[1]) == 0);
    expr[1].depth=0;
    CuAssertTrue(tc, cmpexpr(&expr[0], &expr[1]) == 1);
}


void __test__sanity_1_plus_1(CuTest* tc)
{
    static char * line = "1 + 1";
    static enum Lexicon symbols[] = {INTEGER, ADD, INTEGER};
    struct Token tokens[8];
    char msg[256];

    struct Expr expr;    
    struct Expr answer;
    struct Expr answer_expr[2];
    
    for (int i=0; 2 > i; i++)
        build_int_value_T(&answer_expr[i], i+1);

    answer.type=BinExprT,
    answer.inner.bin.lhs = &answer_expr[0];
    answer.inner.bin.rhs = &answer_expr[1];
    answer.inner.bin.op = Add;

    usize ntokens = tokenize(line, tokens, 0);

    sprintf(msg, "Expected [int, plus, int], instead got '%s %s %s'",
        ptoken(tokens[0].token),
        ptoken(tokens[1].token),
        ptoken(tokens[2].token)
    );

	CuAssertTrue(tc, ntokens == 3);
	CuAssert(
        tc, 
        msg,
        __check_tokens(tokens, symbols, 3) == 0
    );

    memset(msg, 0, 255);

    int ret = construct_expr(line, tokens, 3, &expr) == 0;
    if (ret == 0) {
        printf("Expected: \n");
        ptree(&answer);
        printf("got: \n");
        ptree(&expr);
    }
    CuAssertTrue(tc, ret == 1);
}

void __test__1_plus_2(CuTest* tc) {
    static char * line = "1 + 2";
    struct Expr expr[8];
    struct Token tokens[16];
    usize ntokens;


    build_int_value_T(&expr[0], 1);
    build_int_value_T(&expr[1], 2);
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);

    ntokens = tokenize(line, tokens, 0);
    
    CuAssertTrue(tc, ntokens == 3);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}

int __test__single_perthensis_1(CuTest* tc) {
    struct Expr expr[8];
    struct Token tokens[16];
    static char * line="(1) + 2";        // 5
    usize ntokens;

    build_int_value_T(&expr[0], 1);
    expr[0].depth=1;

    build_int_value_T(&expr[1], 2);
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);

    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 5);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}

void __test__single_perthensis_2(CuTest* tc) {
    struct Expr expr[8];
    struct Token tokens[32];
    char *line = "1 + (2)";        // 5
    usize ntokens;

    build_int_value_T(&expr[0], 1);
    build_int_value_T(&expr[1], 2);
    expr[1].depth=1;
    
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
    ntokens = tokenize(line, tokens, 0);
    CuAssertTrue(tc, ntokens == 5);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}

void __test__single_perthensis_3(CuTest* tc) {
    static char * line = "(1) + (2)";
    struct Token tokens[16];
    struct Expr expr[8];

    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    expr[0].depth=1;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=1;
    
    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    
    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 7);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}

void __test__single_perthensis_4(CuTest* tc) {
    struct Expr expr[8];
    struct Token tokens[32];
    char *line = "(1 + 2)";        // 5
    usize ntokens;
    
    build_int_value_T(&expr[0], 1);
    build_int_value_T(&expr[1], 2);
    expr[0].depth=1;
    expr[1].depth=1;

    build_bin_expr_T(&expr[2], &expr[0], &expr[1], Add);
    expr[2].depth=1;
    
    ntokens = tokenize(line, tokens, 0);
    CuAssertTrue(tc, ntokens == 5);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[3]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[2], &expr[3]) == 1);
}

void __test__single_perthensis_5(CuTest* tc) {
    struct Expr expr[8];
    struct Token tokens[32];
    static char *line = "(1 + 2) + 3";    // 7
    usize ntokens;

    build_int_value_T(&expr[0], 1);
    expr[0].depth=1;

    build_int_value_T(&expr[1], 2);
    expr[1].depth=1;
    
    build_int_value_T(&expr[2], 3);

    build_bin_expr_T(&expr[3], &expr[0], &expr[1], Add);
    expr[3].depth=1;

    build_bin_expr_T(&expr[4], &expr[3], &expr[2], Add);

    ntokens = tokenize(line, tokens, 0);
    CuAssertTrue(tc, ntokens == 7);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);

}

void __test__single_perthensis_6(CuTest* tc) {
    struct Expr expr[8];
    struct Token tokens[32];
    static char *line = "1 + (2 + 3)";    // 7
    usize ntokens;

    build_int_value_T(&expr[0], 1);
    build_int_value_T(&expr[1], 2);
    expr[1].depth=1;

    build_int_value_T(&expr[2], 3);
    expr[2].depth=1;

    build_bin_expr_T(&expr[3], &expr[1], &expr[2], Add);
    expr[3].depth=1;

    build_bin_expr_T(&expr[4], &expr[1], &expr[3], Add);

    ntokens = tokenize(line, tokens, 0);

    CuAssertTrue(tc, ntokens == 7);
    CuAssertTrue(tc, construct_expr(line, tokens, ntokens, &expr[5]) == 0);
    CuAssertTrue(tc, cmpexpr(&expr[4], &expr[5]) == 1);

}

CuSuite* ExprUnitTestSuite(void) {
	CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, __test__sanity_expr_cmp);
	SUITE_ADD_TEST(suite, __test__sanity_test_1);
	SUITE_ADD_TEST(suite, __test__sanity_1_plus_1);
	SUITE_ADD_TEST(suite, __test__1_plus_2);

	SUITE_ADD_TEST(suite, __test__single_perthensis_1);
	SUITE_ADD_TEST(suite, __test__single_perthensis_2);
    SUITE_ADD_TEST(suite, __test__single_perthensis_3);
    SUITE_ADD_TEST(suite, __test__single_perthensis_4);
    SUITE_ADD_TEST(suite, __test__single_perthensis_5);
    SUITE_ADD_TEST(suite, __test__single_perthensis_6);
    
	SUITE_ADD_TEST(suite, __test__double_perthensis_1);
	SUITE_ADD_TEST(suite, __test__double_perthensis_2);
    SUITE_ADD_TEST(suite, __test__double_perthensis_3);
    SUITE_ADD_TEST(suite, __test__double_perthensis_4);
    SUITE_ADD_TEST(suite, __test__double_perthensis_5);
    SUITE_ADD_TEST(suite, __test__double_perthensis_6);
    SUITE_ADD_TEST(suite, __test__double_perthensis_7);
	SUITE_ADD_TEST(suite, __test__double_perthensis_8);
    SUITE_ADD_TEST(suite, __test__double_perthensis_9);
    SUITE_ADD_TEST(suite, __test__double_perthensis_10);
    SUITE_ADD_TEST(suite, __test__double_perthensis_11);
    SUITE_ADD_TEST(suite, __test__double_perthensis_12);
    SUITE_ADD_TEST(suite, __test__double_perthensis_13);
    SUITE_ADD_TEST(suite, __test__double_perthensis_14);
    SUITE_ADD_TEST(suite, __test__double_perthensis_15);
	SUITE_ADD_TEST(suite, __test__double_perthensis_16);
    SUITE_ADD_TEST(suite, __test__double_perthensis_17);

    return suite;
}

#endif
