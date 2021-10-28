
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

    /* ([]|a)[N:N:N] */
    /* cannot index SetT*/
    IdxAccess,

    /* {a:b, a:c}[K] */
    MapAccess,

    /**/
    UndefinedOp = 255
};

enum GroupT {
    // {1:2, 3:4}
    MapT,
    
    // [1, 2]
    ListT,
    
    // (a, b)
    TupleT,

    // {a, b}
    SetT
};

#define GROUPING_SZ 64
struct Grouping { 
    enum GroupT type;
    usize capacity;
    usize length;
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

struct FnCallExpr {
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
    struct Expr * operand;
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
        struct FnCallExpr fncall;
        struct BinExpr bin;
        struct NotExpr not_;
        struct IdxExpr idx;

    } inner;
};


#define FLAG_ERROR 65535
typedef uint16_t FLAG_T; 

/* word */
#define EXPECTING_SYMBOL         2

/* 123 */
#define EXPECTING_INTEGER        4

/* "string" */
#define EXPECTING_STRING         8

/* +/-*^ */
#define EXPECTING_ARITHMETIC_OPERATOR 16

/* . */
#define EXPECTING_APPLY_OPERATOR 32

/* [ */
#define EXPECTING_OPEN_BRACKET   64   
/* ] */
#define EXPECTING_CLOSE_BRACKET  128

/* ( */
#define EXPECTING_OPEN_PARAM     256    
/* ) */
#define EXPECTING_CLOSE_PARAM    512

/* { */
#define EXPECTING_OPEN_BRACE    1024
/* } */
#define EXPECTING_CLOSE_BRACE    2048

/* : , */
#define EXPECTING_DELIMITER      8192

/* expects another token */
#define EXPECTING_NEXT           4096 

#define CTX_LIST 1
#define CTX_IDX 2
#define CTX_SET 4
#define CTX_TUPLE 8
#define CTX_MAP 16

/* If we ever paniced this flag set for the rest of parsing */
#define STATE_INCOMPLETE          1 

/* if set, we ran into an error that requires us to unwind the expression*/
#define STATE_PANIC               2 

/* if set an error occured disastrous to recover from*/
#define INTERNAL_ERROR            4

/* if set - warning messages are present */
#define STATE_WARNING             8

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

enum GroupOperator {
    GOPUndef = 0,
    GOP_INDEX_ACCESS,
    GOP_APPLY
};

enum MarkerT {
    MarkerTUndef = 0,
    // operators
    _IdxAccess, //
    Apply,  // word|)|]|}(

    TupleGroup, // (x,x)
    ListGroup,  // [x,x]
    MapGroup,   // {x:x}
    SetGroup,   // {x,x}
};

#define DELIM_COMMA 1
#define DELIM_COLON 2

/* used in the group-stack exclusively */
struct Group {
    /* pointer of token in operation stack*/
    //struct Token *operation_ptr;

    // amount of delimiters
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
    uint8_t index_accsess_operand_ctr;

    FLAG_T delimiter;
    FLAG_T expecting_ctx;
    // should be `[` `(` '{' or `0`
    struct Token *origin;

    // INDEX / MAP_ACCESS / APPLY
    enum GroupOperator tag_op;
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
