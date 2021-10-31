
#ifndef _HEADER_EXPR__
#define _HEADER_EXPR__

#include <stdint.h>
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
    
    /* *args[] */
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


#endif
