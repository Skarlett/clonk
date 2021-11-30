
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

    // if(a, ...) x;|{x} (?else) x;|{x}
    IfExprT,

    // def foo(a) x;|{x}
    FuncDefExprT,

    // return x|{x};
    ReturnExprT,

    // binary operation
    // 1 + 2 * foo.max - size_of(list)
    BinaryExprT,
    
    UndefinedExprT
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

    /* logical operator */
    And,
    Or,

    Assign,
    AssignAdd,
    AssignSub,

    /* dot token */
    Access,

    Not,

    BitOr,
    BitAnd,
    BitNot,
    
    BandEql,
    BorEql,
    BnotEql,

    ShiftRight,
    ShiftLeft,

    PipeOp,
    
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
    uint16_t length;
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

struct UnaryExpr {
    enum Operation op;
    struct Expr *operand;
};

struct NotExpr {
    struct Expr *operand;
};

struct IfExpr {
    struct Expr *cond;
    struct Expr *body;
    struct Expr *else_body;
};

struct FnDefExpr {
    struct Expr *signature;
    struct Expr *body;
};

struct ReturnExpr {
    struct Expr *body;
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
        struct ReturnExpr ret;
	struct FnDefExpr func;
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

#define GSTATE_EMPTY             1

#define GSTATE_CTX_DATA_GRP      1 << 1
#define GSTATE_CTX_CODE_GRP      1 << 2
#define GSTATE_CTX_MAP_GRP       1 << 3
#define GSTATE_CTX_IDX           1 << 4

#define GSTATE_CTX_LOCK          1 << 5
#define GSTATE_CTX_NO_DELIM      1 << 6
#define GSTATE_CTX_SHORT_BLOCK   1 << 7

struct Group {
    /*
                   0:           Uninitialized state
        GSTATE_EMPTY:           signify empty grouping,
        GSTATE_CTX_DATA_GRP:    parsing comma seperated data (lists, tuples, sets)
        GSTATE_CTX_CODE_GRP:    parsing a set of instructions/code ( `{ a(); b(); }` )
        GSTATE_CTX_MAP_GRP:     parsing literal datatype map ( `{ a:b, c:d };` )
        GSTATE_CTX_IDX :        parsing index expression `[a:b:c]
        GSTATE_CTX_LOCK :       set an immutable parsing context until this group ends
        GSTATE_OP_APPLY :       after group completion, make into an fncall
        GSTATE_CTX_IF_COND:     parsing an if conditional

    */
    FLAG_T state;
    
    // amount of delimiters
    uint16_t delimiter_cnt;

    // amount of delimiters
    uint16_t expr_cnt;
    
    uint16_t operator_idx;
    // should be `[` `(` '{' or `0`
    struct Token *origin;
    struct Token *last_delim;
};

#define FLAG_ERROR       0

#define STATE_READY      1
#define STATE_INCOMPLETE 1 << 1
#define STATE_PANIC      1 << 2 
#define INTERNAL_ERROR   1 << 3
#define STATE_WARNING    1 << 4

/* if/def/ret/else ends with ; */
#define STATE_SHORT_BLOCK 1 << 5

#define STACK_SZ 512
#define EXP_SZ 32

struct ExprParserState {
    struct Token *src;
    uint16_t src_sz;
    uint16_t *_i;
    char * line;

    /* Tree construction happens in this stack */
    struct Expr *expr_stack[STACK_SZ];
    
    /* a stack of pending operations (see shunting yard) */
    struct Token *operator_stack[STACK_SZ];
    
    /* tracks opening braces in the operator stack */
    struct Group set_stack[STACK_SZ];
    
    /* tracks opening braces in the operator stack */
    // struct Group prev_set_stack[16];

    uint16_t set_ctr;
    uint16_t operators_ctr;
    uint16_t expr_ctr;
    
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

    enum Lexicon expecting[EXP_SZ];
    enum Lexicon *expecting_ref;
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
    uint16_t expr_size,
    struct ExprParserState *state,
    struct Expr *ret
);

int8_t free_state(struct ExprParserState *state);
int8_t reset_state(struct ExprParserState *state);
#endif

