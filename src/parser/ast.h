
// #ifndef _HEADER__AST__
// #define _HEADER__AST__
// #include <stdlib.h>

// #include "../prelude.h"

// #include "lexer/lexer.h"
// #include "expr/expr.h"

// #define STMT_CAPACITY 255
// enum StatementType {
//     Undefined,

//     // define function (named block)
//     Define,
    
//     // declare var
//     Declare,

//     //Math Expression (with variables)
//     // or function calls,
//     // + all variables evaluate into expressions
//     Expression,

//     // conditional block
//     Condition,

//     Block,

//     Return
// };


// // typedef enum AssignmentOp {
// //     /* no operation */
// //     AssignmentNop,
// //     Eq,
// //     EqAdd,
// //     EqSub
// //     /* cmp */
// // } AssignmentOp;


// typedef struct Statement {
//     enum StatementType type;
//     void *internal_data;
// } Statement;

// typedef struct BlockStatement {
//     uint32_t capacity;
//     uint32_t length;
//     struct Statement *statements[STMT_CAPACITY];
// } BlockStatement;

// void init_block(struct BlockStatement *block, uint32_t capacity);
// void append_statement(BlockStatement *block, struct Statement *stmt);
// int is_block(struct Token tokens[], uint32_t nstmt);


// typedef struct ReturnStatement {
//     struct Expr * value;
// } ReturnStatement;
// int is_return_statement(char *line, struct Token tokens[], uint32_t nstmt);
// int construct_ret_statement(char *line, struct Token tokens[], uint32_t nstmt, struct Statement *stmt);

// typedef struct FunctionDefinition {
//     uint32_t name_sz;
//     uint32_t param_sz;

//     char func_name[STR_STACK_SIZE];
//     char *parameters[FUNC_ARG_SIZE];
// } FunctionDefinition;

// void init_func_def(struct FunctionDefinition *fn);
// int is_func_definition(char *line, struct Token tokens[], uint32_t nstmt);
// int construct_func_definition(char *line, struct Token tokens[], uint32_t nstmt, struct Statement *stmt);


// typedef struct DeclareStatement {
//     Expr *data;
//     uint32_t name_sz;
//     char name[STR_STACK_SIZE];
// } DeclareStatement;
// int is_declare_statement(struct Token tokens[], int ntokens);
// int construct_declare_statement(char *line, struct Token tokens[], uint32_t nstmt, struct Statement *stmt);

// enum ConditionState {
//     If,
//     Elif,
//     Else
// };

// typedef struct ConditionalStatement {
//     Expr *expr;
//     enum ConditionState state;
// } ConditionalStatement;
// int is_conditional_definition(char *line, struct Token tokens[], uint32_t nstmt);
// void init_condition_stmt(struct ConditionalStatement *stmt);


// typedef struct ExprStatement {
//     Expr *expr;
// } ExprStatement;


// const char * pstmt_type(Statement *stmt);
// int pnode(Statement *stmt, short unsigned indent);
// void print_ast(BlockStatement *tree);
// void print_ast_block(BlockStatement *tree, short unsigned indent);

// //int construct_statement(char *line, struct Token tokens[], uint32_t nstmt, struct BlockStatement *block);
// uint32_t assemble_ast(char *line, Token tokens[], uint32_t ntokens, BlockStatement *block, int *trap);


// #endif
