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
    
    // import x.y, z, ..pkg.b;
    // import a;
    // import ..a;
    // import ..a.b;
    // from . import a, b
    // from module.x import x.x
    ImportExprT,

    // binary operation
    // 1 + 2 * foo.max - size_of(list)
    BinaryExprT,
    
    // !a ~a
    UnaryExprT,

    UndefinedExprT,
    GroupT
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

enum Group_t {
    GroupTUninit,

    // [1, 2]
    ListT,

    // [1:2:3]
    IndexAccessT,

    // (a, b)
    TupleT,

    // {
    PartialBrace,

    //{1:2, 3:4}
    MapT,

    // {1; 2;}
    CodeBlockT
};

struct GroupExpr { 
    enum Group_t type;
    uint16_t length;
    struct Expr **ptr;
};

enum DataType {
    DT_UndefT,
    DT_IntT,
    DT_StringT,
    DT_BoolT,
    DT_CollectionT,
    DT_NullT
};

struct Literals {
    union {
        isize integer;
        char * string;
        uint8_t boolean;
    } literal;
};

struct FnCallExpr {
    char * func_name;
    struct Expr *caller;

    /* TupleGroup(N) expr */
    struct Expr *args;
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

struct IfExpr {
    struct Expr *cond;
    struct Expr *body;
    struct Expr *else_body;
};

struct FnDefExpr {
    const char * name; 
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


/* can be a single token or a string of congruent tokens*/
enum ExprSizeType {
    /* singular */
    _ExprSzTy_Singlar,
    
    /* multiple */
    _ExprSzTy_Span
};

struct Expr {
    enum ExprType type;
    enum DataType datatype;
    // ****

    union {
        struct Token unit;
        struct TokenSpan span; 
    } origin;

    //struct Token origin; 
    // ****
    uint8_t free;

    union {
        char * symbol;
        struct Literals value;
        struct FnCallExpr fncall;
        struct BinExpr bin;
        struct UnaryExpr unary;
        struct IdxExpr idx;
        struct IfExpr cond;
        struct ReturnExpr ret;
        struct FnDefExpr func;
        struct GroupExpr grp;
    } inner;
};


typedef uint16_t FLAG_T; 

enum PrevisionerModeT {
  /* give list of next possible 
   * tokens based on the input */
  PV_Default,
  
  /* follow a sequence of 
   * tokens until completed */  
  PV_DefSignature,
  PV_Import
};

/* Predicts the next possible tokens
 * from the current token.
 * Used to check for unexpected tokens.
 * functionality is
*/

#define PREVISION_SZ 64
union PrevisionerData {
  // default mode
  struct {
    enum Lexicon *ref;
  } default_mode;

  struct {
    uint16_t ctr;
  } fndef_mode;

  struct {
    bool has_word;
    bool expecting_junction;
  } import_mode;
};


// TODO: if top of operator stack has 0 precedense,
// you can push ret/if/else/import
struct Previsioner
{
  enum Lexicon buffer[PREVISION_SZ];
  enum PrevisionerModeT mode;
  union PrevisionerData data;
};

void init_expect_buffer(struct Previsioner *state);

/* void unset_flag(FLAG_T *set, FLAG_T flag); */
/* void set_flag(FLAG_T *set, FLAG_T flag); */
/* FLAG_T check_flag(FLAG_T set, FLAG_T flag); */


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

#define GSTATE_CTX_IDX           1 << 4

#define GSTATE_CTX_LOCK          1 << 5
#define GSTATE_CTX_NO_DELIM      1 << 6
#define GSTATE_CTX_SHORT_BLOCK   1 << 7

struct ParserInput {
    const char * src_code;
    uint16_t src_code_sz;
    struct Vec tokens;

    bool add_glob_scope;
};

struct ParserOutput {
    /* Vec<struct Token *> */
    const struct Vec postfix;

    /* Vec<struct Token> */
    struct Vec token_pool;

    /* Vec<struct ParseError> */
    struct Vec errors;
};

enum ShortBlock_t {
  sh_udef_t,
  sh_cond_t,
  sh_import_t
};


struct Group {
    //TODO: Implement this
    uint16_t seq;
    /*
                   0:           Uninitialized state
        GSTATE_EMPTY:           signify empty grouping,

        xxGSTATE_CTX_DATA_GRP:    parsing comma seperated data (lists, tuples, sets)
        xxGSTATE_CTX_CODE_GRP:    parsing a set of instructions/code ( `{ a(); b(); }` )
        xxGSTATE_CTX_MAP_GRP:     parsing literal datatype map ( `{ a:b, c:d };` )

        GSTATE_CTX_IDX :        parsing index expression `[a:b:c]
        GSTATE_CTX_LOCK :       set an immutable parsing context until this group ends

        xxGSTATE_OP_APPLY :       after group completion, make into an fncall
        xxGSTATE_CTX_IF_COND:     parsing an if conditional

    */
    FLAG_T state;
    
    // amount of delimiters
    uint16_t delimiter_cnt;

    /* 
    ** amount of expressions
     * to consume off the stack 
     *****************
     * TODO: since its unreliable 
     * to measure the amount of delimiters
     * and use that to determine 
     * how many elements to take of the stack.
     ****
     * We will have to keep a counter,
     * expressions such as 
     *   * inner groups,
     *   * if statements,
     *   * function defining
     * should count as 1 expressions, since 
     * thats how they're represented in the stack 
    ** this is essential to ensure code-blocks work
    */
    uint16_t expr_cnt;

    /*
    ** where the grouping is inside
    ** of the operator stack
    */
    uint16_t operator_idx;

    enum Group_t type;

    // should be `[` `(` '{' or `0`
    const struct Token *origin;
    const struct Token *last_delim;


    /*
    ** TODO: implement
    ** NOTE: if the amount of if & else are not equal
    **       will cause UB -
    **       don't be a prick, warn the user.
    ** RULES:
    ** if (x) x;
    **         ^ After delimiter, check for `else`
    ** else y;
    **/
    //uint8_t short_block;
    enum ShortBlock_t short_type;

};

#define FLAG_ERROR       0

#define STATE_READY      1
#define STATE_INCOMPLETE 1 << 1
#define STATE_PANIC      1 << 2
#define INTERNAL_ERROR   1 << 3

/* 
  if set - there is an extra 
    OPEN_BRACK inside of the operator stack 
    and will be popped off at the EOF
*/
#define STATE_PUSH_GLOB_SCOPE 1 << 4

/* if/def/ret/else ends with ; */
// #define STATE_SHORT_BLOCK 1 << 5

#define STACK_SZ 512
#define EXP_SZ 32

enum ParserError_t {
    parse_err_unexpected_token,
};

enum ErrTok_t {
  ET_Scalar,
  ET_Span
};

struct UnexpectedTokError {
    /* Lexicon[N] null-terminated malloc */
    enum Lexicon *expected;
    uint16_t nexpected;

    struct TokenSelection selection;
};


/* used to construct an error */
struct PartialError {
    enum ParserError_t type;
    struct Token start;

    /*NOTE: needs free*/
    enum Lexicon *expect;
    uint16_t nexpected;
};


struct ParserError {
    enum ParserError_t type;
    struct TokenSelection unwind_window;

    union {
        struct UnexpectedTokError unexpected_tok;

    } error;
};

/*
** restoration works by destroying a
** portion of the upper part of the stack.
**
** It will slice the top (newest) portion of the stack
** mark INCOMPLETE, and continue.
*/
struct RestorationFrame {
    /* points to storation point  */
    const struct Token * operator_stack_tok;
    const struct Token * output_tok;
    const struct Token * current;
    const struct Group * grp;
};


struct PostfixStageState {

    /* Vec<struct Expr> */
    struct Vec pool;

    struct Expr * stack[STACK_SZ];
    uint16_t stack_ctr;
    uint16_t *_i;
};

struct GroupBookKeeper {
    uint16_t next_id;

    /* Vec<struct GroupBooklet> */
    struct Vec tabs;
};

struct GroupBooklet {
    struct Token *start;
    struct Token *end;
};


struct Parser {
    const struct Token *src;
    const char * src_code;
    uint16_t src_sz;
    uint16_t *_i;

    /* a stack of pending operations (see shunting yard) */
    const struct Token *operator_stack[STACK_SZ];

    /* tracks opening braces in the operator stack */
    struct Group set_stack[STACK_SZ];

    /* tracks opening braces in the operator stack */
    // struct Group prev_set_stack[16];

    uint16_t set_ctr;
    uint16_t operators_ctr;

    uint16_t operator_stack_sz;
    uint16_t set_sz;

    /* todo, to get group spans */
    struct GroupBookKeeper grp_keeper;

    /*
    ** Keep generated tokens in `pool`.
    ** Generated meaning they were not previously
    ** created in the previous stage (lexing)
    */
    /*Vec<struct Token>*/
    struct Vec pool;

    /* Vec<struct Token *> */
    struct Vec debug;

    /*Vec<struct ParseError>*/
    struct Vec errors;

    struct Previsioner expecting;

    /*Vec<struct RestorationFrame>*/
    struct Vec restoration_stack;
    uint16_t restoration_ctr;

    /* whenever panic is set a
     * partial_err is valid, and
     * caused on previous loop */
    bool panic;
    struct PartialError partial_err;


    /* enabled if parser cannot
     * move to the next stage */
    bool stage_failed;
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
int8_t parse(
    struct ParserInput *input,
    struct ParserOutput *out
);

int8_t is_token_unexpected(struct Parser *state);

int8_t free_state(struct Parser *state);
int8_t reset_state(struct Parser *state);

// void mk_null(struct Expr *ex);

// int8_t mk_str(struct PostfixStageState *state, struct Expr *ex); 
// int8_t mk_int(struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_symbol(struct PostfixStageState *state, struct Expr *ex);

// int8_t mk_operator(struct PostfixStageState *state, struct Expr *ex, struct Token *op_head);
// int8_t mk_group(struct PostfixStageState *state, struct Expr *ex);

// int8_t mk_binop(struct Token *op, struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_not(struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_idx_access(struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_fncall(struct PostfixStageState *state, struct Expr *ex);

// enum Operation operation_from_token(enum Lexicon token);
// void determine_return_ty(struct Expr *bin);

// int8_t mk_if_cond(struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_if_body(struct PostfixStageState *state);
// int8_t mk_else_body(struct PostfixStageState *state);

// int8_t mk_return(struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_def_sig(struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_def_body(struct PostfixStageState *state);
// int8_t mk_import(struct PostfixStageState *state, struct Expr *ex);
void restoration_hook(struct Parser *state);
int8_t handle_unwind(
    struct Parser *state,
    bool unexpected_token
);

void throw_unexpected_token(
  struct Parser *state,
  const struct Token *start,
  enum Lexicon expected[],
  uint16_t nexpected
);

#endif
