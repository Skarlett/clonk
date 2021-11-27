
#ifndef _HEADER_EXPR__
#define _HEADER_EXPR__

#include <stdint.h>
#include "../../utils/vec.h"
#include "../../prelude.h"
#include "../lexer/lexer.h"

/*
    when defining a group, 
    it may not have more literal elements than
*/
enum ExprType {
    NopExprT,
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

    // x(a, ...)
    IfExprT,


    // binary operation
    // 1 + 2 * foo.max - size_of(list)
    BinaryExprT,
    
    UndefinedExprT
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

    Assign,
    AssignAdd,
    AssignSub,

    /* dot operator */
    Access,

    Not,

    UndefinedOp = 255
};

enum GroupT {
    GroupTUndef,

    // {1:2, 3:4}
    MapT,
    
    // [1, 2]
    ListT,
    
    // (a, b)
    TupleT,

    // {a, b}
    SetT,

    // {1; 2;}
    CodeBlockT

};

struct Grouping { 
    enum GroupT type;
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
    struct Expr **args;
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

struct IfExpr {
    struct Expr *cond;
    struct Expr *body;
    struct Expr *else_body;
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
    struct Token origin; 
    uint8_t free;

    union {
        char * symbol;
        struct Literals value;
        struct FnCallExpr fncall;
        struct BinExpr bin;
        struct NotExpr not_;
        struct IdxExpr idx;
        struct IfExpr cond;

    } inner;
};


typedef uint16_t FLAG_T; 

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
  
    used in the group-stack exclusively
*/


#define GSTATE_EMPTY         1
#define GSTATE_CTX_DATA_GRP  2
#define GSTATE_CTX_CODE_GRP  4
#define GSTATE_CTX_MAP_GRP   8
#define GSTATE_CTX_IDX      16
#define GSTATE_CTX_LOCK     32
#define GSTATE_CTX_NO_DELIM 1024
#define GSTATE_OP_APPLY     64
#define GSTATE_CTX_IF_COND 128
#define GSTATE_CTX_IF_BODY 256
#define GSTATE_CTX_ELSE_BODY 512

struct Cond {
    struct Token *origin;
    // ';' or '{'
    enum Lexicon expecting;
    
    /*
        0 : Uninitialized state
        1 : Condition Completed,
        2 : Body Completed,
    */
    uint8_t flags;
};

struct Group {
    /*
        0 : Uninitialized state
        1 : Empty grouping,
        2 : CTX Comma data (lists, tuples, sets)
        4 : CTX Code-block ( {a(); b();} )
        8 : CTX Map mode {a : b};
       16 : CTX Index mode 
       32 : CTX Lock
       64 : apply marker operation
      
      # Not in use yet
      256 : if marker operation
      512 : else marker operation
     1024 : def-body operator
     2048 : def-signature operator
     4092 : s

    */
    FLAG_T state;
    
    // amount of delimiters
    uint16_t delimiter_cnt;

    // should be `[` `(` '{' or `0`
    struct Token *origin;
    struct Token *last_delim;
};

#define FLAG_ERROR                0
#define STATE_READY               1
#define STATE_INCOMPLETE          2
#define STATE_PANIC               4 
#define INTERNAL_ERROR            8
#define STATE_WARNING             16

#define STATE_CTX_ERROR 0
#define STATE_CTX_DEFAULT 1

#define STATE_CTX_RETURN_BODY 2
#define STATE_CTX_IF_HEAD 4
#define STATE_CTX_IF_BODY 8
#define STATE_CTX_ACCEPT_ELSE 16
#define STATE_CTX_ELSE_BODY 32


#define STACK_SZ                 512
struct ExprParserState {
    struct Token *src;
    usize src_sz;
    usize *_i;
    char * line;

    /* Tree construction happens in this stack */
    struct Expr *expr_stack[STACK_SZ];
    
    /* a stack of pending operations (see shunting yard) */
    struct Token *operator_stack[STACK_SZ];
    
    /* tracks opening braces in the operator stack */
    struct Group set_stack[STACK_SZ];
    
    struct Cond cond_stack[STACK_SZ];

    /* tracks opening braces in the operator stack */
    struct Group prev_set_stack[16];

    uint16_t set_ctr;
    uint16_t operators_ctr;
    uint16_t expr_ctr;
    uint16_t cond_ctr;
    
    uint16_t expr_sz;
    uint16_t operator_stack_sz;
    uint16_t set_sz;

    /* Vec<struct Expr> */
    struct Vec expr_pool;

    /*Vec<struct Token>*/
    struct Vec pool;
    
    /* Vec<struct Token *> */
    struct Vec debug;

    /*Vec<struct CompileTimeError>*/
    struct Vec errors;

    FLAG_T ctx;

    FLAG_T expecting;
    FLAG_T panic_flags;
};
/*
  Shunting yard expression parsing algorthim 
  https://en.wikipedia.org/wiki/Shunting-yard_algorithm
  --------------

  This function takes takes a stream of token `tokens[]`
  and writes an array of pointers (of type `struct Token`)
  into `*output[]` in postfix notation.

  The contents of `*output[]` will be a
  POSTFIX notation referenced from the 
  INFIX notation of `input[]`.

    infix: 1 + 1
  postfix: 1 1 +
    input: [INT, ADD, INT]
   output: [*INT, *INT, *ADD]

  Further more, this function handles organizing operation precedense
  based on shunting-yard algorthm.
  This is in combination with arithmetic operations, and our custom operations
  (GROUP, INDEX_ACCESS, APPLY, DOT).
  
  Upon completion, the result will be an ordered array of operands, 
  and operators ready to be evaluated into a tree structure.

    infix: (1+2) * (1 + 1)
  postfix: 1 2 + 1 1 + *
  To turn the output into a tree see `stage_postfix_parser`.

  Digging deeper into the realm of this, 
  you'll find I evaluate some custom operators
  such as the DOT token, and provide 
  extra operators to the output to describe
  function calls (APPLY(N)).

  infix:   foo(a, b+c).bar(1)
  postfix  foo a b c + APPLY(3) bar 1 APPLY(2) .
  pretty-postfix:
           ((foo a (b c +) APPLY(3)) bar 1 APPLY(2) .)
*/
int8_t parse_expr(
    char * line,
    struct Token tokens[],
    usize expr_size,
    struct ExprParserState *state,
    struct Expr *ret
);

int8_t free_state(struct ExprParserState *state);
int8_t reset_state(struct ExprParserState *state);
#endif
