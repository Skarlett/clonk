#ifndef _HEADER__EXPR__
#define _HEADER__EXPR__

#define STR_STACK_SIZE 32
#define FUNC_ARG_SIZE 8
#include <stdlib.h>

typedef enum Tag {
    NullTag,
    VariableTag,
    ValueTag
} Tag;

typedef enum DataType {
    NullT,
    IntT,
    StringT,
} DataType;


// variable OR literal
typedef struct Value {
    void *data_ptr;
    DataType datatype;
} Value;

// variable OR literal
typedef struct Variable {
    char word[STR_STACK_SIZE];
} Variable;

// variable OR literal
typedef struct Symbol {
    void *data_ptr;
    // variable or 
    Tag tag;
} Symbol;


void init_symbol(Symbol *v);

typedef enum ExprType {
    UndefinedExprT,
    UniExprT,
    BinExprT,
} ExprType;

typedef struct Expr {
    ExprType type;
    // void wi
    void *inner_data;
} Expr;
int print_expr(Expr *expr, short unsigned indent);
void init_expression(struct Expr *expr);
int is_expr(char *line, struct Token tokens[], size_t ntokens);

typedef struct FunctionCallExpr {
    int name_sz;
    int args_sz;

    char *func_name; 
    Expr *args[FUNC_ARG_SIZE];
} FunctionCallExpr;

int unit_from_token(char *line, struct Token token, struct Symbol *value);
void init_func_call(struct FunctionCallExpr *fn);
int is_func_call(struct Token tokens[], int nstmt);


typedef enum UniaryOperation {
    UniaryOperationNop,
    Call,
    UniValue
} UniaryOperation;

typedef struct UniExpr {
    enum UniaryOperation op;

    // Value or Call struct
    void *inner;
} UniExpr;

int init_uni_expr_body(struct UniExpr *expr);
int unit_into_uniary(struct Symbol *val, struct UniExpr *expr);

typedef enum BinOp {
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
} BinOp;

typedef struct BinExpr {
    enum BinOp op;
    struct Expr left_val;
    struct Expr right_val;
} BinExpr;

void init_bin_expr_body(struct BinExpr *expr);
/* ------------------------------------------ */
/*             generic expression struct      */
/* ------------------------------------------ */
// orchestrate symbols/values into expressions...

int construct_expr(char *line, struct Token tokens[], unsigned long  ntokens, struct Expr *expr);
#endif