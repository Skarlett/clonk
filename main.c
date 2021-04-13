// CLONK interpreter
// my very own 1990s retro built interpreter
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lexer.h"
#include "expr.h"

#define TRUE 1
#define FALSE 0
#define EXTENSION "ka"

#define STR_STACK_SIZE 64
#define FUNC_ARG_SIZE 8
#define STMT_CAPACITY 16




/* ------------------------------------------ */
/*            Func call                       */
/* ------------------------------------------ */


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


struct Statement {
    enum StatementType type;
    void *internal_data;
};



//
// struct ExprStatement {
//     struct Symbol base;
//     enum BinOperation op;
//     struct ExprStatement *other;
// };

// void init_expr_stmt(struct ExprStatement *expr) {
//     expr->op=Nop;
//     expr->other=0;
//     init_unit(&expr->base);
// }

// int is_express_statement(struct Token tokens[], int nstmt) {
//     if (nstmt-1 % 2 == 0)
//         return 0; 
    
//     for (int i=0; nstmt-1 > i; i++) {
//         if (tokens[i].token != SUB || tokens[i].token != ADD)
//             return 0;
//     }

//     return 1;
// }

// int inner_expr_stmt(char *line, struct Token tokens[], int nstmt, struct ExprStatement *ex_stmt){
//     struct ExprStatement *expr = malloc(sizeof(struct ExprStatement));
//     init_expr_stmt(expr);
//     //free(expr->base.data_ptr);
//     unit_from_token(line, tokens[0], &expr->base);
    
//     if (nstmt > 1) {
//         if (tokens[1].token == ADD)
//             expr->op=Add;
//         else if (tokens[1].token == SUB)
//             expr->op=Sub;
//         else
//             return -1;
    
//         struct ExprStatement *o_expr = malloc(sizeof(struct ExprStatement));
//         init_expr_stmt(o_expr);

//         inner_expr_stmt(line, tokens+2, nstmt-2, o_expr);
//         expr->other = o_expr;
//         return 0;
//     }
//     return 0;
// }

// int construct_expr_stmt(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
//     struct ExprStatement *expr = malloc(sizeof(struct ExprStatement));
    
//     init_expr_stmt(expr);
//     inner_expr_stmt(line, tokens, nstmt, expr);

//     stmt->internal_data=expr;
//     stmt->type=Expression;
//     return 0;
// }



/* ------------------------------------------ */
/*            Block statement                 */
/* ------------------------------------------ */


struct BlockStatement {
    struct Statement *statements;
    unsigned long capacity;
    unsigned long length;
};

void init_block(struct BlockStatement *block, unsigned long capacity) {
    block->statements = malloc(sizeof(struct Statement)*capacity);
    block->capacity=capacity;
    block->length=0;
}

void append_statement(struct BlockStatement *block, struct Statement stmt) {
    if (block->length >= block->capacity) {
        block->capacity *= 2;
        block->statements=realloc(block->statements, block->capacity);
    }

    block->length += 1;
    block->statements[block->length] = stmt;
}


// loop where we collect into a block
//

// or we recurse into it (preferred)
int is_block(struct Token tokens[], int nstmt) {
   return (tokens[nstmt].token == CLOSE_BRACK) && (tokens[0].token == OPEN_BRACE);
}


/* ------------------------------------------ */
/*            return                          */
/* ------------------------------------------ */


struct ReturnStatement {
    struct Symbol value;
};

int is_return_statement(char *line, struct Token tokens[], int nstmt) {
    for (int i=0; 6 > i; i++)
        if ((tokens[0].start + line)[i] != "return"[i])
            return FALSE;
    
    return TRUE;
}

void construct_ret_statement(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
    struct Symbol var;
    struct ReturnStatement *ret_stmt = malloc(sizeof(struct ReturnStatement));
    unit_from_token(line, tokens[1], &var);
    stmt->internal_data=ret_stmt;
    stmt->type=Return;
}

struct DeclareStatement {
    int name_sz;
    char name[STR_STACK_SIZE];
    struct Symbol data;
};

// word open param [expression, ...] close param

int construct_func_call(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
    struct FunctionCallExpr *fn_stmt = malloc(sizeof(struct FunctionCallExpr));
    // open_param
    long unsigned oparam_idx = 0;
    long unsigned cparam_idx = 0;
    
    fn_stmt->name_sz = 0;
    fn_stmt->args_sz = 0;

    for (int i=0; FUNC_ARG_SIZE > i; i++){
        //init_unit(&fn_stmt->args[i]);
    }
    
    // setup name
    fn_stmt->name_sz=tokens[0].end - tokens[0].start;

    char *fname = malloc(fn_stmt->name_sz);
    
    strncpy(fname, tokens[0].start + line, fn_stmt->name_sz);
    fn_stmt->func_name = fname;

    
    // no parameters
    if (nstmt == 4) {
        //printf("no parameters in '%s'", fn_stmt->func_name);
        return 0;
    }
    unsigned long last_expr = 2;

    while (nstmt-2 > last_expr) {
        unsigned long expr_idx = 0;

        for (int i=last_expr; nstmt-1 > i; i++) {
            if (tokens[i].token == COMMA) {
                expr_idx = i;
                break;
            }
        }

        if (expr_idx == 1) {

        }

        else if (is_func_call(tokens + last_expr, expr_idx)) {

        }

        //parse_expression(line, tokens + las);
    }
    //TODO construc
    //construct arguments
    for (int i=2; nstmt-2 > i; i++) {
        if (tokens[i].token == COMMA) continue;

        struct Symbol base = {
            .datatype=Null,
            .data_ptr=0,
            .tag=Literal
        };
        
        if (unit_from_token(line, tokens[i], &base) != 0) {
            fprintf(stderr, "error in unit_from_token");
            exit(1);
        }
        
        if (fn_stmt->args_sz > FUNC_ARG_SIZE)
            return -1;
        
        //fn_stmt->args[fn_stmt->args_sz] = base;
        fn_stmt->args_sz += 1;
    }
    
    stmt->internal_data=fn_stmt;
    //stmt->type=CallFunc;

    return 0;
}
/* ------------------------------------------ */
/*            Func def                        */
/* ------------------------------------------ */

struct FunctionDefinition {
    int name_sz;
    int param_sz;

    char func_name[STR_STACK_SIZE];
    char parameters[FUNC_ARG_SIZE];
};

void init_func_def(struct FunctionDefinition *fn) {
    struct BlockStatement body;
    init_block(&body, STMT_CAPACITY);

    fn->name_sz = 0;
    fn->param_sz = 0;

    for (int i=0; FUNC_ARG_SIZE > i; i++){
        fn->parameters[i] = 0;
    }
}

// word('def') word open_param ... close_param
int is_func_definition(char *line, struct Token tokens[], int nstmt) {
    if (tokens[0].token == WORD) {
        for (int i=0; 3 > i; i++)
            if ((line + tokens[0].start)[i] != "def"[i]) 
                return 0;
        

        return tokens[1].token == WORD  
                && tokens[2].token == PARAM_OPEN
                && tokens[nstmt].token == PARAM_CLOSE;
    } else
        return 0;
}

int construct_func_definition(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
    printf("declaring func...\n");
    struct FunctionDefinition *proceedure = malloc(sizeof(struct FunctionDefinition));
    init_func_def(proceedure);
    strncpy(proceedure->func_name, (line + tokens[1].start), tokens[1].end);
    

    for (int i=2; nstmt-1 > i; i++) {
        if (tokens[i].token == COMMA) continue;
        char *parameter = malloc(tokens[i].end - tokens[i].start);
        strncpy(parameter, line + tokens[i].start, tokens[i].end);

        if (proceedure->param_sz > FUNC_ARG_SIZE)
            return -1;
        
        //proceedure->parameters[proceedure->param_sz] = parameter;
        proceedure->param_sz += 1;
    }

    stmt->internal_data = proceedure;
    stmt->type = Define;

    return 0;
}




/* ------------------------------------------ */
/*            if statement                    */
/* ------------------------------------------ */

enum ConditionState {
    If,
    Elif,
    Else
};

// struct ConditionStatement {
//     struct ExprStatement expr;
//     enum ConditionState state;
// //    struct BlockStatement if_true;
// //    struct BlockStatement if_false;
// };

// void init_condition_stmt(struct ConditionStatement *stmt) {
//     struct BlockStatement block;
//     stmt->state=If;
//     init_expr_stmt(&stmt->expr);
// }

// // word('if') open param expr close param
// int is_conditional_definition(char *line, struct Token tokens[], int nstmt) {
//     char keyword[4];
//     memset(keyword, 0, 4);

//     for (int i=0; 4 > i; i++) {
//         keyword[i] = (line + tokens[0].start)[i];
//     }
    
//     return tokens[0].token == WORD
//     && (strcmp(keyword, "if")
//         || strcmp(keyword, "elif")
//         || strcmp(keyword, "else"))
//     && tokens[1].token == PARAM_OPEN
//     && tokens[nstmt].token == PARAM_CLOSE;
// }


// int construct_condition_statement(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
//     struct ConditionStatement *condition_stmt = malloc(sizeof(struct ConditionStatement));
//     init_condition_stmt(condition_stmt);

//     struct Statement *expr_stmt = malloc(sizeof(struct Statement));
//     construct_expr_stmt(line, tokens+2, nstmt-1, expr_stmt);

//     stmt->internal_data = condition_stmt;
//     stmt->type = Condition;

//     return 0;
// }


/* ------------------------------------------ */
/*            declare variable                */
/* ------------------------------------------ */

// x = 200
int is_declare_statement(struct Token tokens[], int ntokens) {
    return ntokens == 4
    && tokens[0].token == WORD
    && tokens[1].token == EQUAL
    && (
        tokens[2].token == STRING_LITERAL
        || tokens[2].token == INTEGER
        || tokens[2].token == WORD
    );
}

int construct_declare_statement(char *line, struct Token tokens[], struct Statement *stmt) {
    struct DeclareStatement *dec_stmt = malloc(sizeof(struct DeclareStatement));
    struct Symbol data;
    int name_len = 0;
    dec_stmt->name_sz = 0;
    init_unit(&dec_stmt->data);
    
    for (int i=0; 3 > i; i++) {
        if (i == 1)
            continue;

        else if (i == 0 && tokens[i].token != WORD)
            return -1;

        else if (i==0) {
            char *slice = (line + tokens[i].start);

            int c=0;
            for(;tokens[i].end > c ;c++)
                dec_stmt->name[c] = slice[c];
            
            dec_stmt->name_sz=c;
        }

        // copy identifier
        else if (i == 2) {
            if (unit_from_token(line, tokens[i], &data) != 0) {
                fprintf(stderr, "error in unit_from_token");
                exit(1);
            }
            dec_stmt->data=data;
        }
    }
    // poo, more heap allocations
    stmt->internal_data = dec_stmt;
    stmt->type = Declare;
    return 0;
}


/* ------------------------------------------ */
/*            construct all statements        */
/* ------------------------------------------ */


int construct_statement(char *line, struct Token tokens[], long unsigned nstmt, struct BlockStatement *block) {
    // 2 + 2
    struct Statement new;
    
    for (int i=0; nstmt > i; i++) 
        printf("[%s] ", ptoken(tokens[i].token));
    printf("\n");
    if (line == 0) return 0;
    if (nstmt == 0) return -1;

    // if (is_express_statement(tokens, nstmt))
    //     construct_expr_stmt(line, tokens, nstmt, &new);
    
    // declare var
    else if (is_declare_statement(tokens, nstmt))
        construct_declare_statement(line, tokens, &new);
   
    // return statement
    else if (is_return_statement(line, tokens, nstmt))
        construct_ret_statement(line, tokens, nstmt, &new);

    // foo( ... )
    else if (is_func_call(tokens, nstmt)) 
        construct_func_call(line, tokens, nstmt, &new);
    
    // def foo((T),*)
    else if (is_func_definition(line, tokens, nstmt)) {
        //*expects_next = Block;
        construct_func_definition(line, tokens, nstmt, &new);
    }

    // // if ( expr )
    // else if (is_conditional_definition(line, tokens, nstmt)) {
    //     //*expects_next = Block;
    //     construct_condition_statement(line, tokens, nstmt, &new);
    // }
    
    else if (tokens[0].token == OPEN_BRACE) {
        struct BlockStatement new_block;
        init_block(&new_block, STMT_CAPACITY);
        
        construct_statement(line, tokens + 1, nstmt, &new_block);
        new.type = Block;
        new.internal_data = &new_block;
    }
    

    else {
        char *slice = malloc(tokens[nstmt].end - tokens[0].start);
        strncpy(slice, (line + tokens[0].start), tokens[nstmt].end);

        printf("cannot parse out raw data ,\n```\n%s\n```\n", slice);
        return -1;
    }
    append_statement(block, new);
    return 0;
}

int assemble_ast(
    char *line,
    struct Token tokens[],
    long unsigned ntokens,
    struct BlockStatement *block)
{
    unsigned long last_stmt_idx = 0;
    unsigned long statement_idx = 0;

    while(ntokens > last_stmt_idx){
    
        for(long unsigned i=last_stmt_idx; ntokens > i; i++) {
            enum Lexicon token = tokens[i].token;

            if (token == SEMICOLON )
            {
                statement_idx = i+1;
                break;
            }
        }
        printf("tokens: tokens[%d..%d] [%d] -- ", (int)last_stmt_idx, (int)statement_idx, (int)ntokens);
        unsigned long slen = statement_idx-last_stmt_idx;

        if (construct_statement(line, tokens + last_stmt_idx, slen, block) != 0)
            return -1;
        
        last_stmt_idx = statement_idx;
    }
    return 0;
}


int parse(char *filepath) {
    FILE *fd;
    struct BlockStatement root;
    init_block(&root, STMT_CAPACITY*2);

    unsigned long n_completed = 0;
    int buf_sz = 2048;
    struct Token tokens[buf_sz];
    long unsigned token_n = 0;
    char line[buf_sz];
    memset(line, 0, buf_sz);
    
    if ((fd = fopen(filepath, "r")) == NULL) {
        perror("Error! opening file");
        exit(1);
    }

    int n = 1;

    while (n > 0) {
        fread(line, sizeof(char), buf_sz, fd);

        //calculate the index/position of the last character written to the buffer
        for (long int i=0; buf_sz > i; i++) {
            if (line[i] == 0) {
                n=i;
                break;
            }
        }

        if (n <= 0) {
            break;
        }

        int ntokens = tokenize(line, tokens, token_n);
        
        printf("token stream: ");
        for (int i=0; ntokens > i; i++) {
            printf("[%s(%d,%d)] ", ptoken(tokens[i].token), tokens[i].start, tokens[i].end);
        }
        
        printf("\n\n");
        printf("----------------\n");
        printf("AST\n");
        printf("----------------\n");
        assemble_ast(line, tokens, ntokens, &root);

        printf("\n");
        memset(line, 0, buf_sz);
        n_completed = 0;
    }

    fclose(fd);    
    return 0;
}


int main(int argc, char *argv[]) {
    if (argc > 1) {
        return parse(argv[1]);
    }
    else {
        printf("%s [file.%s]\n", argv[0], EXTENSION);
        return 1;
    }
}
