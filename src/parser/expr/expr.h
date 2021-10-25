
#ifndef _HEADER_EXPR__
#define _HEADER_EXPR__

#include <stdint.h>
#include "../../prelude.h"
#include "../lexer/lexer.h"

/*
    when defining a group, 
    it may not have more literal elements than
*/
#define MAX_GROUP_SZ 256
#define MAX_ARGS_SZ 24
#define MAX_FUNC_NAME_SZ 256

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

    
    // binary operation
    // 1 + 2 * foo.max - size_of(list)
    BinaryExprT

};

enum Operation {
    Nop,
    /* no operation */
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
    
    /* dot operator */
    Access,
    

    Not,
    /* L[N:N:N] */
    IdxAccess,

    UndefinedOp = 255
};

#define GROUPING_SZ 64
struct Grouping { 
    usize capacity;
    usize length;
    enum Lexicon brace;
    struct Expr **ptr;
};

enum DataType {
    UndefT,
    IntT,
    StringT,
    BoolT,
    GroupT,
    NullT
};

struct Literals {
    union {
        isize integer;
        char * string;
        uint8_t boolean;
        struct Grouping grouping;
    } literal;
};


struct CallExpr {
    struct Expr *caller;
    char * func_name;
    uint8_t args_length;
    struct Expr *args[MAX_ARGS_SZ];
    enum DataType returns;
};

struct BinExpr {
    enum Operation op;
    struct Expr *lhs;
    struct Expr *rhs;
    enum DataType returns;
};

struct NotExpr {
    struct Expr *operand;
};

struct IdxExpr {
    struct Expr *operand;
    struct Expr * start;
    struct Expr * end;
    struct Expr * skip;
};

struct Expr {
    enum ExprType type;
    enum DataType datatype;
    uint8_t free;

    union {
        char * symbol;
        struct Literals value;
        struct CallExpr fncall;
        struct BinExpr bin;
        struct NotExpr not_;
        struct IdxExpr idx;

    } inner;
};


#define FLAG_ERROR 65535
typedef uint16_t FLAG_T; 


/* if bit set, expects operand to be the next token */
#define EXPECTING_SYMBOL         2

#define EXPECTING_INTEGER        4

#define EXPECTING_STRING         8

/* if bit set, expects binary operator to be the next token */
#define EXPECTING_OPERATOR       16

/* if bit set, expects opening brace type be the next token */
#define EXPECTING_OPEN_BRACKET   32   

/* if bit set, expects closing brace type be the next token */
#define EXPECTING_CLOSE_BRACKET  64

/* if bit set, expects opening brace type be the next token */
#define EXPECTING_OPEN_PARAM     128    

/* if bit set, expects closing brace type be the next token */
#define EXPECTING_CLOSE_PARAM    256

/* if bit set, expects a comma be the next token */
#define EXPECTING_COMMA          512   

/* if bit set, expects a colon until bracket_brace token type is closed */
#define EXPECTING_COLON          1024 

/* hint that colons are acceptable */
#define _EX_COLON_APPLICABLE     2048 

/* if bit set, expects a token to follow */
#define EXPECTING_NEXT           4096 


/* If token stream contains an error */
/* we'll attempt to recover, by discarding tokens */
#define STATE_PANIC               1 

/* If we ever paniced */
/* we'll attempt to recover, by discarding tokens, 
but keep this flag set for the rest of parsing */
#define STATE_INCOMPLETE          2 

/* too disastrous to recover from */
#define INTERNAL_ERROR            4 

/*
    The grouping stack is used to track the amount 
    of sub-expressions inside an expression. (See lexer.h)
    We generate GROUPING tokens based on the stack model
    our parser uses.

    For every new brace token-type added into the operator-stack
    increment the grouping stack, and initalize it to 0.
    For every comma, increment the current grouping-stack's head by 1.
    
    Once the closing brace is found and
    this stack's head is larger than 0,
    we have a set/grouping of expressions. 
*/
struct Group {
    struct Token *postfix_group_token;

    /* pointer of token in operation stack*/
    //struct Token *operation_ptr;

    // amount of delimiters + 1
    uint16_t delimiter_cnt;
    
    // count symbols inside of group
    // lamen terms, count every 
    // token between braces
    uint16_t atomic_symbols;

    // count how many values have
    // been placed in the output
    // to account for the index/slice 
    // operation arguments.
    // This is so we know how many NULLs 
    // to place
    uint8_t arg_align_ctr;

    // should be ',' ':' or `0`
    enum Lexicon delimiter;

    // should be `[` `(` or `0`
    struct Token *origin;

    // INDEX / APPLY
    enum Lexicon tag_op;
};

struct StageInfixState {
    struct Token *src;
    usize src_sz;
    usize *i;

    struct Token **out;
    usize out_sz;
    usize *out_ctr;

    /*
      quick bump-allocator pool
      all tokens created in this stage will be
      referenced from this pool
    */
    struct Token *pool;
    usize pool_i;
    usize pool_sz;

    struct Group *set_stack;
    usize set_ctr;
    usize set_sz;

    struct Token **operator_stack;
    uint16_t operators_ctr;
    uint16_t operator_stack_sz;

    FLAG_T expecting;
    FLAG_T state;
};

int8_t postfix_expr(
    struct Token *tokens[],
    usize expr_size,
    
    struct Token *output[],
    usize output_sz,
    usize *output_ctr,
    
    struct StageInfixState *state,
    struct CompileTimeError *err
);

#endif
