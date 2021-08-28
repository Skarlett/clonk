#ifndef _HEADER__EXPR__
#define _HEADER__EXPR__

#define STR_STACK_SIZE 32
#define FUNC_ARG_SIZE 8

#include <stdint.h>
#include <sys/types.h>
#include <stdlib.h>
#include "../lexer.h"

typedef enum Tag {
    UndefinedTag,
    NullTag,
    VariableTag,
    ValueTag
} Tag;

typedef enum DataType {
    UndefinedDataType,
    NullT,
    IntT,
    StringT,
    BoolT
} DataType;

typedef enum ExprType {
    UndefinedExprT,
    UniExprT,
    BinExprT
} ExprType;

typedef enum UnitaryOperation {
    UniOpNop,
    UniCall,
    UniValue
} UnitaryOperation;

typedef enum BinOp {
    /* no operation */
    BinOpNop,
    /* math */
    Add,
    Sub,
    Multiply,
    Divide,
    Pow,
    Modolus,
    /* cmp */
    IsEq,
    NotEq,
    Gt,
    Lt,
    GtEq,
    LtEq,
    /* appendage */
    And,
    Or
} BinOp;

typedef enum AssignmentOp {
    /* no operation */
    AssignmentNop,
    Eq,
    EqAdd,
    EqSub
    /* cmp */
} AssignmentOp;


typedef struct InteralString { 
    uint32_t capacity;
    uint32_t length;
    char * ptr;
} InteralString;


typedef struct InternalData {
    enum DataType type;
    union {
        uint8_t boolean;
        int64_t integer;
        struct InteralString string;
    } data;
} InternalData;


typedef struct Symbol {
    Tag tag;
    // flags const,static,dyn,ro,rw,rwx
    union {
        char * variable;
        struct InternalData value;
    } inner;
} Symbol;


int symbol_from_token(char *line, struct Token token, struct Symbol *value);

/*
It seemed like a good idea using Unions as opaque types at the time. 
I woefully regret this now.
*/
typedef struct Expr {
    enum ExprType type;
    
    union {
        struct {
            enum UnitaryOperation op;
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

int is_func_call(struct Token tokens[], int nstmt);
int construct_expr(char *line, struct Token tokens[], unsigned long  ntokens, struct Expr *expr);

#endif