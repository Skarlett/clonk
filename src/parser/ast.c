#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "lexer.h"
#include "../common.h"
#include "expr.h"
#include "ast.h"



/* ------------------------------------------ */
/*            Block statement                 */
/* ------------------------------------------ */


void init_block(BlockStatement *block, size_t capacity) {
    memset(block->statements, 0, STMT_CAPACITY);
    block->capacity=capacity;
    block->length=0;
}

void append_statement(BlockStatement *block, Statement *stmt) {
    // TODO Dynamic sizing
    block->statements[block->length] = stmt;
    block->length += 1;
}


// loop where we collect into a block
//

// or we recurse into it (preferred)
int is_block(Token tokens[], size_t nstmt) {
   return (tokens[nstmt].token == CLOSE_BRACE) && (tokens[0].token == OPEN_BRACE);
}

/* ------------------------------------------ */
/*            return                          */
/* ------------------------------------------ */

int is_return_statement(char *line, Token tokens[], size_t nstmt) {
    for (int i=0; 6 > i; i++)
        if ((tokens[0].start + line)[i] != "return"[i])
            return FALSE;
    
    return TRUE;
}

int construct_ret_statement(char *line, Token tokens[], size_t nstmt, Statement *stmt) {
    Expr *expr = malloc(sizeof(Expr));

    ReturnStatement *ret_stmt = xmalloc(sizeof(ReturnStatement));

    construct_expr(line, tokens + 2, nstmt, expr);
    ret_stmt->value=expr;
    stmt->internal_data=ret_stmt;
    stmt->type=Return;
    return 0;
}


/* ------------------------------------------ */
/*            func Def                        */
/* ------------------------------------------ */
void init_func_def(FunctionDefinition *fn) {
    BlockStatement body;
    init_block(&body, STMT_CAPACITY);

    fn->name_sz = 0;
    fn->param_sz = 0;

    for (int i=0; FUNC_ARG_SIZE > i; i++){
        fn->parameters[i] = 0;
    }
}

// word('def') word open_param ... close_param
int is_func_definition(char *line, Token tokens[], size_t nstmt) {
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

int construct_func_definition(char *line, Token tokens[], size_t nstmt, Statement *stmt) {
    printf("declaring func...\n");
    FunctionDefinition *proceedure = xmalloc(sizeof(FunctionDefinition));
    init_func_def(proceedure);
    strncpy(proceedure->func_name, (line + tokens[1].start), tokens[1].end);
    

    for (size_t i=2; nstmt-1 > i; i++) {
        if (tokens[i].token == COMMA) continue;
        char *parameter = xmalloc(tokens[i].end - tokens[i].start);
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


int construct_expr_stmt(char *line, Token tokens[], size_t nstmt, Statement *stmt) {
    ExprStatement *expr_stmt = xmalloc(sizeof(ExprStatement));
    Expr *expr = xmalloc(sizeof(Expr));
    
    if (construct_expr(line, tokens, nstmt, expr) != 0)
        return -1;
    
    expr_stmt->expr=expr;
    stmt->internal_data=expr_stmt;
    stmt->type=Expression;
    return 0;
}



/* ------------------------------------------ */
/*            if statement                    */
/* ------------------------------------------ */

void init_condition_stmt(ConditionalStatement *stmt) {
    BlockStatement block;
    stmt->state=If;
}

// word('if') open param expr close param
int is_conditional_definition(char *line, Token tokens[], size_t nstmt) {
    char keyword[4];
    memset(keyword, 0, 4);

    for (int i=0; 4 > i; i++) {
        keyword[i] = (line + tokens[0].start)[i];
    }
    
    return tokens[0].token == WORD
    && (strncmp(keyword, "if", 2) == 0
        || strcmp(keyword, "elif") == 0
        || strcmp(keyword, "else") == 0)
    && tokens[1].token == PARAM_OPEN
    && tokens[nstmt-1].token == PARAM_CLOSE;
}

int construct_conditional(char *line, Token tokens[], size_t nstmt, Statement *stmt) {
    ConditionalStatement *con_stmt = xmalloc(sizeof(ConditionalStatement));
    Expr *expr = malloc(sizeof(Expr));
    char keyword[4];

    stmt->type=Condition;
    stmt->internal_data=con_stmt;
    con_stmt->expr=expr;

    memset(keyword, 0, 4);

    for (int i=0; 4 > i; i++) {
        keyword[i] = (line + tokens[0].start)[i];
    }
    
    if (strncmp(keyword, "if", 2) == 0) con_stmt->state=If;
    else if (strcmp(keyword, "elif") == 0) con_stmt->state=Elif;
    else if (strcmp(keyword, "else") == 0) con_stmt->state=Else;
    else return -1;
    
    construct_expr(line, tokens + 2, nstmt, expr);
    return 0;
}

/* ------------------------------------------ */
/*            declare variable                */
/* ------------------------------------------ */

// x = 200

int is_declare_statement(Token tokens[], int ntokens) {
    return tokens[0].token == WORD
    && tokens[1].token == EQUAL;

}

int construct_declare_statement(char *line, Token tokens[], size_t nstmt, Statement *stmt) {
    Expr *expr = xmalloc(sizeof(Expr));
    DeclareStatement *dec_stmt = xmalloc(sizeof(DeclareStatement));
    
    dec_stmt->name_sz = tokens[0].end - tokens[0].start;
    construct_expr(line, tokens + 2,  nstmt, expr);

    if (STR_STACK_SIZE >= dec_stmt->name_sz) {
        strncpy(dec_stmt->name, line + tokens[0].start, dec_stmt->name_sz);
    }

    dec_stmt->data=expr;
    stmt->internal_data = dec_stmt;
    stmt->type = Declare;
    return 0;
}



const char * pstmt_type(Statement *stmt) {
    switch (stmt->type) {
        case Undefined: return "undefined";
        case Define: return "func def";
        case Declare: return "var declaration";
        case Expression: return "expression";
        case Block: return "block";
        case Condition: return "if-else";
        case Return: return "return";
        default: return "UNKNOWN STATEMENTTYPE";
    }
}

/* ------------------------------------------ */
/*            construct all statements        */
/* ------------------------------------------ */


int construct_statement(char *line, Token tokens[], size_t nstmt, BlockStatement *block) {
    // 2 + 2
    Statement *stmt = xmalloc(sizeof(Statement));
    stmt->internal_data=0;
    stmt->type=Undefined;
   
    if (line == 0) return 0;
    else if (nstmt == 0) return -1;

    // return statement
    else if (is_return_statement(line, tokens, nstmt)) {
        if (construct_ret_statement(line, tokens, nstmt, stmt) != 0) {
            printf("error in ret def");
        }
    }
    
    //  if ( expr )
    else if (is_conditional_definition(line, tokens, nstmt)) {
        //*expects_next = Block;
        if (construct_conditional(line, tokens, nstmt, stmt) != 0) {
            printf("error in conditional def\n");
        }
    }

    // declare var
    else if (is_declare_statement(tokens, nstmt)) {
        if (construct_declare_statement(line, tokens, nstmt, stmt) != 0) {
            printf("error in declare var");
        }
    }


    // def foo((T),*)
    else if (is_func_definition(line, tokens, nstmt)) {
        //*expects_next = Block;
        if (construct_func_definition(line, tokens, nstmt, stmt) != 0) {
            printf("error in func def\n");
        }
    }
    
    // func();
    // a == b;
    else if (is_expr(line, tokens, nstmt)) {
        if (construct_expr_stmt(line, tokens, nstmt, stmt) != 0) {
            printf("error in expr def\n");
        }
    }

    else {
        char *slice = xmalloc(tokens[nstmt].end - tokens[0].start);
        strncpy(slice, (line + tokens[0].start), tokens[nstmt].end);

        printf("cannot parse out raw data ,\n```\n%s\n```\n", slice);
        return -1;
    }

    if (stmt->type == Undefined || stmt->internal_data == 0) {
        printf("bad stuff happened");
        exit(1);
    }

    append_statement(block, stmt);
    return 0;
}

// returns the number of tokens consumed

size_t assemble_ast(
    char *line,
    Token tokens[],
    size_t ntokens,
    BlockStatement *block,
    int *depth
){
    Statement *child = NULL;
    size_t 
        last_stmt_idx = 0,
        statement_idx = 0,
        ctr = 0,
        skip = 0;


    while(ntokens > statement_idx){
        for(size_t i=last_stmt_idx; ntokens > i; i++) {
            
            enum Lexicon token = tokens[i].token;
            if (skip > 0) {
                skip -= 1;
                last_stmt_idx += 1;
                continue;
            }

            else if (token == SEMICOLON )
            {
                statement_idx = i+1;
                break;
            }

            else if (token == OPEN_BRACE) {
                BlockStatement *child_block = xmalloc(sizeof(BlockStatement));
                child = xmalloc(sizeof(Statement));
                child->type = Block;
                child->internal_data = child_block;
                init_block(child_block, STMT_CAPACITY);
                // TODO: we have to deal with partial loads eventually
                // god only knows how we'll restore the child
                // as the current block
                //--------------------------------------
                *depth += 1;
                // this function will go on and look ahead
                // so when we get `skip` it will be how many 
                // tokens it seeked ahead, and constructed.
                //---
                // the depth parameter is being used
                // as a way to pass an error up the chain
                skip += assemble_ast(line, tokens + i + 1, ntokens, child_block, depth) + 1;
                *depth -= 1;
                // if (*depth == -101)
                //     return ctr;
                
                statement_idx = i;
                break;
            }

            else if (token == CLOSE_BRACE) return ctr+1;
            ctr++;
        }

        printf("statement: tokens[%d..%d] [total: %d] [depth: %d] (skip: %d) --  ", (int)last_stmt_idx, (int)statement_idx, (int)ntokens, *depth, skip);
        size_t slen = statement_idx-last_stmt_idx;
        
        
        for (size_t p_i=0; slen > p_i; p_i++) 
            printf("[%s] ", ptoken((tokens + last_stmt_idx)[p_i].token));
        printf("\n");

        if (construct_statement(line, tokens + last_stmt_idx, slen, block) != 0)
            return 0;
        
        if (child != 0) {
            append_statement(block, child);
            child = NULL;
        }

        last_stmt_idx = statement_idx;
    }
    return ctr;
}
char * pcondition_state(enum ConditionState c) {
    switch(c) {
        case If: return "if";
        case Elif: return "elif";
        case Else: return "else";
        default: return "UNKNOWN CONDITION";
    }
}

int pnode(Statement *stmt, short unsigned indent){
    tab_print(indent);
    printf("{\n");
    indent++;
    tab_print(indent);
    printf("statement type: %s\n", pstmt_type(stmt));
    tab_print(indent);
    printf("data: {\n");
    indent++;
    if (stmt->type == Undefined) return -1;
    
    else if(stmt->type == Define) {
        FunctionDefinition *data = stmt->internal_data;
        
        printf("name: %s\n", data->func_name);
        tab_print(indent);
        printf("parameters: [");

        for (int i=0; data->param_sz > i; i++) {
            printf("%s", data->parameters[i]);
            if (i != data->param_sz-1)
                printf(", ");
        }
        printf("];\n");
    }
    
    else if(stmt->type == Declare) {
        DeclareStatement *data = stmt->internal_data;
        tab_print(indent);
        printf("name: %s\n", data->name);
        tab_print(indent);
        printf("expression: {\n");
        print_expr(data->data, indent+1);
        tab_print(indent);
        printf("}\n");
    }
    
    else if(stmt->type == Condition) {
        ConditionalStatement *data = stmt->internal_data;
        tab_print(indent);
        printf("keyword: %s\n", pcondition_state(data->state));
        tab_print(indent);
        printf("expr: {\n");
        print_expr(data->expr, indent+1);
        tab_print(indent);
        printf("}\n");
    }

    else if(stmt->type == Return) {
        ReturnStatement *data = stmt->internal_data;
        if (print_expr(data->value, indent) != 0) {
            return -1;
        }
    }
    
    else if(stmt->type == Expression) {    
        ExprStatement *expr_stmt = stmt->internal_data;
        print_expr(expr_stmt->expr, indent);
    }

    tab_print(indent-1); 
    printf("}\n");
    tab_print(indent-2);
    printf("}");
    return 0;
}

void print_ast_block(BlockStatement *tree, short unsigned indent) {
    tab_print(indent);
    printf("{[\n");
    for (int i=0; tree->length > i; i++) {
        if (tree->statements[i]->type == Block)
            print_ast_block(tree->statements[i]->internal_data, indent+1);
        else 
            pnode(tree->statements[i], indent+1);
        
        if (tree->length-1 != i)
            putc(',', stdout);
        putc('\n', stdout);
    }
    tab_print(indent);
    printf("]}");
}



void print_ast(BlockStatement *tree) {
    short unsigned indent = 1;
    printf("{[\n");
    
    for (int i=0; tree->length > i; i++) {
        if (tree->statements[i]->type == Block) 
            print_ast_block(tree->statements[i]->internal_data, indent);
        else
            pnode(tree->statements[i], indent);
        
        if (tree->length-1 != i)
            putc(',', stdout);
        putc('\n', stdout);
    }
    printf("]}\n");
}
