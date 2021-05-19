#ifndef _HEADER__EXPR__
#define _HEADER__EXPR__

#define STR_STACK_SIZE 32
#define FUNC_ARG_SIZE 8

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>

typedef enum Tag {
    NullTag,
    VariableTag,
    ValueTag,
} Tag;

typedef enum DataType {
    NullT,
    IntT,
    StringT,
} DataType;

typedef enum ExprType {
    UndefinedExprT,
    UniExprT,
    BinExprT,
} ExprType;


typedef enum UniaryOperation {
    UniaryOperationNop,
    UniCall,
    UniValue
} UniaryOperation;



typedef enum BinOp {
    // no operation
    BinaryOperationNop,
    // math
    Add,
    Sub,
    Multiply,
    Divide,
    Pow,
    Modolus,
    // cmp
    IsEq,
    NotEq,
    Gt,
    Lt,
    GtEq,
    LtEq,
    // appendage
    And,
    Or,
} BinOp;


void init_bin_expr_body(struct BinExpr *expr);
/* ------------------------------------------ */
/*             generic expression struct      */
/* ------------------------------------------ */
// orchestrate symbols/values into expressions...


typedef struct InteralString { 
    uint32_t capacity;
    uint32_t length;
    char * ptr;
} InteralString;


typedef struct InternalData {
    enum DataType type;
    union {
        int32_t integer;
        struct InteralString string;
    } data;
} InternalData;


typedef struct Symbol {
    Tag tag;
    
    union {
        char * variable;
        struct InternalData value;
    } inner;
} Symbol;


int symbol_from_token(char *line, struct Token token, struct Symbol *value);
int is_func_call(struct Token tokens[], int nstmt);


typedef struct Expr {
    enum ExprType type;
    uint32_t depth;
    
    union {
        struct {
            enum UniaryOperation op;
            union {
                struct Symbol symbol;
                struct {
                        char *func_name; 
                        uint16_t name_capacity;
                        uint16_t name_length;

                        uint16_t args_capacity;
                        uint16_t args_length;
                        struct Expr **args;

                } fncall;
            } interal_data;

        } uni;

        struct {
            enum BinOp op;
            struct Expr *lhs;
            struct Expr *rhs;
        } bin;

    } inner;
} Expr;

int construct_expr(char *line, struct Token tokens[], unsigned long  ntokens, struct Expr *expr);

int print_expr(Expr *expr, short unsigned indent);
void init_expression(struct Expr *expr);
int is_expr(char *line, struct Token tokens[], size_t ntokens);
#endif