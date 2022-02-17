#ifndef __HEADER_AST__
#define __HEADER_AST__

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

    WhileLoopExprT,
    ForLoopExprT,

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

// enum Operation operation_from_token(enum onk_lexicon_t token);
// void determine_return_ty(struct Expr *bin);

// int8_t mk_if_cond(struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_if_body(struct PostfixStageState *state);
// int8_t mk_else_body(struct PostfixStageState *state);

// int8_t mk_return(struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_def_sig(struct PostfixStageState *state, struct Expr *ex);
// int8_t mk_def_body(struct PostfixStageState *state);
// int8_t mk_import(struct PostfixStageState *state, struct Expr *ex);
#endif
