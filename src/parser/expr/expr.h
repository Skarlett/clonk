
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

enum MarkerT {
    // operators
    MarkerNop = 0,
    
    /*
      INDEX_ACCESS acts as a function in the postfix representation
      that takes 4 arugments off the stack
      'source, start, end, skip' in that order.
      
      when INDEX_ACCESS arguments maybe padded with NULLTOKENS
      inserted by the first stage or the user.
      NULLTOKENS when parsed into expression trees 
      will assume their value based on position except
      for the first argument (the source being index).
      The 2nd argument (start) will assume 0 if NULLTOKEN is present.
      The 3rd argument (end) will assume the length of the array if NULLTOKEN is present.
      The 4th (skip) will assume 1 if NULLTOKEN is present.
  
      Examples:
        token output: WORD   INTEGER INTEGER INTEGER INDEX_ACCESS 
                      source start   end     skip    operator
                text: foo[1::2]
          postfix-IR: foo 1 NULL 2 INDEX_ACCESS
    */
    _IdxAccess, 

    // foo(a)(b)
    // foo(a) -> func(b) -> T
    // foo(a)(b) -> ((a foo), b G(2)) DyCall
    // foo(a)(b)(c) -> a foo b G(2) DyCall c G(2) DyCall
    // word|)|]|}(
    Apply,

    /*
      GROUPING token are generated in the expression parser
        
      The GROUPING token is used to track the amount 
      of sub-expressions inside an expression.
        
      - `start` 
          points to its origin grouping token,
          or 0 if not applicable
        
      - `end`
          is the amount of arguments to pop from the stack

      See example expression:
          [1, 2, 3]
      
          where '1', '2', '3' are atomic expressions.
          When grouped together by a comma to become 
          "1,2,3" this is called grouping.

          Groupings may not explicitly 
          point to a brace type if none is present. 
            
          After postfix evaluation, 
          a group token is added into the postfix output.

          1 2 3 GROUP(3)
        
      it's `start` attribute will point to its origin grouping token
      and its `end` attribute will determine the amount of arguments
      it will take off of the stack
    
      tokens of this type will be spawned from a differing source
      than the original token stream.
    */
    TupleGroup, // (x,x)
    ListGroup,  // [x,x]
    MapGroup,   // {x:x}
    SetGroup,   // {x,x}

    MarkerTUndef = 255,
};

#define GSTATE_CTX_LIST 4
#define GSTATE_CTX_TUPLE 8
#define GSTATE_CTX_SET 16
#define GSTATE_CTX_MAP 32
#define GSTATE_CTX_IDX 64
#define GSTATE_CTX_LOCK 128
#define GSTATE_OP_IDX 256
#define GSTATE_OP_APPLY 512
#define GSTATE_OP_GROUP 1024

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
    // : , ctx(5) idx apply

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
