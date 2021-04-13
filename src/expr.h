#ifndef _HEADER__EXPR__
#define _HEADER__EXPR__

#define STR_STACK_SIZE 64
#define STMT_CAPACITY 16
#define FUNC_ARG_SIZE 8

typedef enum Tag {
    Variable,
    Literal
} Tag;

typedef enum DataType {
    Null,
    Int,
    String,
} DataType;

// variable OR literal
typedef struct Symbol {
    void *data_ptr;
    enum Tag tag;
    enum DataType datatype;
} Symbol;

void init_symbol(struct Symbol *v);

typedef enum ExprType {
    UndefinedExpr,
    UniExpr,
    BinExpr,
} ExprType;

typedef struct Expr {
    enum ExprType type;
    // void wi
    void *inner_data;
} Expr;

void init_expression(struct Expr *expr);


typedef struct FunctionCallExpr {
    int name_sz;
    int args_sz;

    char *func_name; 
    struct Expr *args[FUNC_ARG_SIZE];
} FunctionCallExpr;

int unit_from_token(char *line, struct Token token, struct Symbol *value);
void init_func_call(struct FunctionCallExpr *fn);
int is_func_call(struct Token tokens[], int nstmt);


typedef enum UniaryOperation {
    UniaryOperationNop,
    Call,
    Value
} UniaryOperation;

typedef struct UniaryExpr {
    enum UniaryOperation op;
    // Value or Call struct
    void *inner;
} UniaryExpr;

void init_uni_expr_body(struct UniaryExpr *expr);
int unit_into_uniary(struct Symbol *val, struct UniaryExpr *expr);

typedef enum BinaryOperation {
    // no operation
    BinaryOperationNop,
    // math
    Add,
    Sub,
    Multiply,
    Divide,
    Pow,
    Modolus,
    // cmp
    IsEq,
    Gt,
    Lt,
    GtEq,
    LtEq,
    // appendage
    And,
    Or,
} BinaryOperation;

typedef struct BinaryExprBody {
    enum BinaryOperation op;
    struct Expr *left_val;
    struct Expr *right_val;
} BinaryExprBody;

void init_bin_expr_body(struct BinaryExprBody *expr);
/* ------------------------------------------ */
/*             generic expression struct      */
/* ------------------------------------------ */
// orchestrate symbols/values into expressions...

int construct_expr(char *line, struct Token tokens[], unsigned long  ntokens, struct Expr *expr);
#endif