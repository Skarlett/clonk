
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
    Not,
    
    /* dot operator */
    Access,

    /* L[N:N:N] */
    IdxAccess,


    UndefinedOp = 255
};

struct String { 
    usize capacity;
    usize length;
    char * ptr;
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
    ListT,
    NullT
};

struct Literals {
    union {
        isize integer;
        struct String string;
        uint8_t boolean;
        struct Grouping grouping;
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
    uint8_t free;

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

#define FLAG_ERROR 65535
typedef uint16_t FLAG_T; 


/* if bit set, expects operand to be the next token */

#define EXPECTING_WORD           2

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
    enum Lexicon open_brace;

    // INDEX / APPLY
    enum Lexicon tag_op;
};



struct ExprParserState {

    /*
        quick bump-allocator pool
        all tokens created in this stage will be
        referenced from this pool
    */
    struct Token *src;
    usize src_sz;
    usize *i;

    struct Token **out;
    usize out_sz;
    usize *out_ctr;

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
    
    struct ExprParserState *state,
    struct CompileTimeError *err
);

#endif
