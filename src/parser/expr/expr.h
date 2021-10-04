
#ifndef _HEADER_EXPR__
#define _HEADER_EXPR__

#include <stdint.h>
#include "../../prelude.h"
#include "../lexer/lexer.h"

enum ExprType {
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

};

enum Operation {
    /* no operation */
    Nop,
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

    And,
    Or,
    Not,
    
    /* dot operator */
    Access,

};

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

struct Literals {
    enum DataType type;
    union {
        isize integer;
        struct String string;
        uint8_t boolean;
        struct List list;
    } literal;
};

struct FnCallNode {
    enum DataType returns;
    char * func_name; 
    uint8_t name_capacity;
    uint8_t name_length;
    uint8_t args_capacity;
    uint8_t args_length;
    struct Expr * args;
};

// /*
//     `head` should always contain a valid reference to an Expr
//     `tail` will contain a valid reference, or a null pointer
// */
// struct LinkedExpr {
//     struct Expr *head;
//     struct Expr *tail;
// };

struct BinExprNode {
    enum Operation op;
    struct Expr *lhs;
    struct Expr *rhs;
    enum DataType returns;
};

struct Expr {
    enum ExprType type;
    enum DataType datatype;

    union {
        char * symbol;
        struct Literals value;
        struct FnCallNode fncall;
        struct BinExprNode bin;
    } inner;
};


struct FnCall {
    struct Token token;
    uint8_t argc;
};


int mk_fnmask_tokens(
    struct Token *output[],
    usize output_sz,
    usize *output_ctr,

    struct Token input[],
    usize expr_size,

    struct Token masks[],
    usize masks_sz,
    usize *masks_ctr,
    struct CompileTimeError *err
);

int8_t postfix_expr(
    struct Token *tokens[],
    usize expr_size,
    struct Token *output[],
    usize output_sz,
    usize *output_ctr,
    struct FnCall fn_map[],
    usize fn_map_sz,
    struct CompileTimeError *err
);

#endif
