
#ifndef _HEADER_EXPR__
#define _HEADER_EXPR__

#include <stdint.h>
#include "../../prelude.h"

typedef enum ExprType {
    UndefinedExprT,
    
    // variable names
    // x
    // foo.max
    // std::int::MAX
    SymExprT,

    // Literal datatype
    // [1, 2, 3] | list
    // "asdasd"
    // 100
    LiteralExprT,

    // x(a, ...)
    FnCallExprT,

    // Chain of evaluation
    // foo(x.map(f)).length
    // FnCall -> Symbol
    LinkedExprT,
    
    // binary operation
    // 1 + 2 * foo.max - size_of(list)
    BinaryExprT
} ExprType;

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

struct String { 
    usize capacity;
    usize length;
    char * ptr;
};

struct List { 
    usize capacity;
    usize length;
    struct Expr *ptr;
};

enum DataType {
    UndefT,
    IntT,
    StringT,
    BoolT,
    ListT,
    StructT,
    NullT
};

struct ConstData {
    enum DataType type;
    union {
        isize integer;
        struct String string;
        uint8_t boolean;
        struct List list;
    } literal;
};

struct FnCall {
    enum DataType returns;
    char * func_name; 
    uint8_t name_capacity;
    uint8_t name_length;
    uint8_t args_capacity;
    uint8_t args_length;
    struct Expr * args;
};

/*
    `head` should always contain a valid reference to an Expr
    `tail` will contain a valid reference, or a null pointer
*/
struct LinkedExpr {
    struct Expr *head;
    struct Expr *tail;
};

struct BinExpr {
    enum BinOp op;
    struct Expr *lhs;
    struct Expr *rhs;
    enum DataType returns;
};

struct Expr {
    enum ExprType type;
    enum DataType datatype;

    union {
        char * symbol;
        struct ConstData value;
        struct FnCall fncall;
        struct BinExpr bin;
    } inner;
};


#endif
