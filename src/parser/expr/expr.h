
#ifndef _HEADER_EXPR__
#define _HEADER_EXPR__

#include <stdint.h>
#include <sys/types.h>
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
    SetT

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


#define GSTATE_EMPTY        1
#define GSTATE_CTX_BLOCK    2

#define GSTATE_CTX_LIST     4
#define GSTATE_CTX_TUPLE    8
#define GSTATE_CTX_SET     16
#define GSTATE_CTX_MAP     32
#define GSTATE_CTX_IDX     64
#define GSTATE_CTX_LOCK   128
#define GSTATE_OP_IDX     256
#define GSTATE_OP_APPLY   512
#define GSTATE_OP_GROUP   1024
#define GSTATE_OP_IF_COND 2048
#define GSTATE_OP_IF_BODY 4092
struct Group {
    // amount of delimiters
    uint16_t delimiter_cnt;

    /*
        0 : Uninitialized state
        1 : COLON token in group
        2 : COMMA token in group
        4 : List context mode
        8 : Tuple context mode
       16 : Set context mode
       32 : Map context mode
       64 : Index context mode
      (?) : Tuple context mode (without bracing)
      128 : lock context mode
      256 : index marker operation
      512 : apply marker operation
     1024 : group marker operation
    */
    FLAG_T state;

    // should be `[` `(` '{' or `0`
    struct Token *origin;
    struct Token *last_delim;
};


#define FLAG_ERROR           0

#define STATE_READY               1
#define STATE_INCOMPLETE          2
#define STATE_PANIC               4 
#define INTERNAL_ERROR            8
#define STATE_WARNING             16

/* if set - warning messages are present */
#define STACK_SZ 512
struct ExprParserState {
    struct Token *src;
    usize src_sz;
    usize *_i;
    char * line;

    struct Expr *expr_stack[STACK_SZ];
    u_int8_t canary_expr;
    
    struct Token *operator_stack[STACK_SZ];
    u_int8_t canary_op;
    
    struct Group set_stack[STACK_SZ];
    u_int8_t canary_set;
    
    usize set_ctr;
    uint16_t operators_ctr;
    usize expr_ctr;

    usize expr_sz;
    uint16_t operator_stack_sz;
    usize set_sz;

    /* Vec<struct Expr> */
    struct Vec expr_pool;

    /*Vec<struct Token>*/
    struct Vec pool;
    
    /* Vec<struct Token *> */
    struct Vec debug;

    /*Vec<struct CompileTimeError>*/
    struct Vec errors;

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
