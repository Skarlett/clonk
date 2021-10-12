
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


/*
    This represents the second stage of parsing function calls.
    the attribute `token` should always be an FNMASK token.
    
    The amount of arguments the function 
    call was made with is represented with `argc`.

    When sorting order precedence, 
    the amount of arguments each 
    function call expects is lost.

    This information is used to recover that after 
    we've sorted our precedence.
*/
struct FnCall {
    struct Token token;
    uint16_t argc;
    uint16_t id;
};


int8_t mk_fnmask_tokens(
    struct Token input[],
    usize expr_size,

    struct Token masks[],
    usize masks_sz,
    usize *masks_ctr,
  
    struct CompileTimeError *err
);

typedef uint16_t FLAG_T; 

struct ExprParserState {

    /* 
        **sets contains references to groups in the output
    */
    struct Token **sets;
    uint16_t sets_sz;
    uint16_t set_ctr;

    /*
        **function_hints contains references to function calls
    */
    struct Token **function_hints;
    uint16_t function_hints_sz;
    uint16_t function_hints_ctr;

    /*
        quick bump-allocator pool
    */
    struct Token *token_pool;
    usize pool_sz;
    usize pool_i;

    FLAG_T flags;

};

/* if bit set, expects operand to be the next token */
#define EXPECTING_OPERAND        1

/* if bit set, expects binary operator to be the next token */
#define EXPECTING_OPERATOR       2

/* if bit set, expects opening brace type be the next token */
#define EXPECTING_OPEN_BRACE     4    

/* if bit set, expects closing brace type be the next token */
#define EXPECTING_CLOSE_BRACE    8

/* if bit set, expects a comma be the next token */
#define EXPECTING_COMMA          16   

/* if bit set, expects a colon until bracket_brace token type is closed */
#define EXPECTING_COLON          32 

/* if bit set, expects a token to follow */
#define EXPECTING_NEXT           64  

/* If token stream contains an error */
/* we'll attempt to recover, by discarding tokens */
#define PANIC_FLAG               128 

/* If we ever paniced */
/* we'll attempt to recover, by discarding tokens */
#define INCOMPLETE_FLAG          256 

int8_t postfix_expr(
    struct Token *tokens[],
    usize expr_size,
    
    struct Token *output[],
    usize output_sz,
    usize *output_ctr,
    
    struct ExprParserState *state,
    struct CompileTimeError *err
);

#endif
