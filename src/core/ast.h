
#ifndef _HEADER__AST__
#define _HEADER__AST__
#include <stdlib.h>
#include "lexer.h"
#include "common.h"
#include "expr.h"

#define STMT_CAPACITY 16
enum StatementType {
    Undefined,

    // define function (named block)
    Define,
    
    // declare var
    Declare,

    //Math Expression (with variables)
    // or function calls,
    // + all variables evaluate into expressions
    Expression,

    // conditional block
    Condition,

    Block,

    Return
};


typedef struct BlockStatement {
    // struct BlockStatement *parent;
    struct Statement **statements;
    size_t capacity;
    size_t length;
} BlockStatement;
void init_block(struct BlockStatement *block, size_t capacity);
void append_statement(BlockStatement *block, struct Statement *stmt);
int is_block(struct Token tokens[], size_t nstmt);


typedef struct ReturnStatement {
    Expr value;
} ReturnStatement;
int is_return_statement(char *line, struct Token tokens[], size_t nstmt);
void construct_ret_statement(char *line, struct Token tokens[], size_t nstmt, struct Statement *stmt);

typedef struct FunctionDefinition {
    size_t name_sz;
    size_t param_sz;

    char func_name[STR_STACK_SIZE];
    char *parameters[FUNC_ARG_SIZE];
} FunctionDefinition;
void init_func_def(struct FunctionDefinition *fn);
int is_func_definition(char *line, struct Token tokens[], size_t nstmt);
int construct_func_definition(char *line, struct Token tokens[], size_t nstmt, struct Statement *stmt);


typedef struct Statement {
    enum StatementType type;
    void *internal_data;
} Statement;

//int construct_statement(char *line, struct Token tokens[], size_t nstmt, struct BlockStatement *block);
int assemble_ast(char *line, Token tokens[], size_t ntokens, BlockStatement *block);





typedef struct DeclareStatement {
    Expr data;
    size_t name_sz;
    char name[STR_STACK_SIZE];
} DeclareStatement;
int is_declare_statement(struct Token tokens[], int ntokens);
int construct_declare_statement(char *line, struct Token tokens[], size_t nstmt, struct Statement *stmt);

enum ConditionState {
    If,
    Elif,
    Else
};

typedef struct ConditionalStatement {
    Expr expr;
    enum ConditionState state;
} ConditionalStatement;
int is_conditional_definition(char *line, struct Token tokens[], size_t nstmt);
void init_condition_stmt(struct ConditionalStatement *stmt);


typedef struct ExprStatement {
    Expr *expr;
} ExprStatement;



const char * pstmt_type(struct Statement *stmt);
int pnode(struct Statement *stmt, short unsigned indent);

#endif