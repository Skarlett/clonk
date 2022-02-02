// CLONK interpreter
// my very own 1990s retro built interpreter
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "parser/lexer/lexer.h"
#include "parser/expr/expr.h"
#include "parser/expr/debug.h"
#include "parser/ast.h"
#include "parser/synthetize.h"
#include "prelude.h"


#define HELP_TEXT \
    "\t -h | brings up this help menu\n" \
    "\t -V | print version\n" \
    "\t -a | prints ast\n" \
    "\t -t | prints token stream\n" \
    "\t --extree | prints expression tree in a simplier format"
   

void print_help(char *name) {
    printf("usage: %s [opts] [file]\nversion: %s\n\n", name, VERSION);
    printf("%s\n", HELP_TEXT);
}

struct Opts {
    uint_fast8_t print_ast;
    uint_fast8_t print_expr_tree;
    uint_fast8_t print_tokens;
};

void init_opts(struct Opts *opts) {
    opts->print_ast=0;
    opts->print_expr_tree=0;
    opts->print_tokens=0;
}

void setup_opts(int argc, char *argv[], struct Opts *opts) {
    int skip = 0;

    for (int i=1; argc > i; i++) {
        if (skip > 0) {
            skip -= 1;
            continue;
        }

        else if (strcmp(argv[i], "-V") == 0) {
            printf("%s\n", VERSION);
            exit(0);
        }

        else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            print_help(argv[0]);
            printf("\n");
            exit(0);
        }

        else if (strcmp(argv[i], "-t") == 0) {
            opts->print_tokens=1;
        }

        else if (strcmp(argv[i], "-a") == 0) {
            opts->print_ast=1;
        }
        else if (strcmp(argv[i], "--extree") == 0) {
            opts->print_expr_tree=1;
        }
        
    }
}



int parse(char * fp, struct Opts *opts) {
    FILE *fd;
    
    struct BlockStatement root;
    init_block(&root, STMT_CAPACITY*2);


    struct Token tokens[2048];

    size_t n_completed = 0;
    size_t buf_sz = 2048;
    size_t token_n = 0;

    char src_code[buf_sz];
    memset(src_code, 0, buf_sz);
    
    if ((fd = fopen(fp, "r")) == NULL) {
        perror("Error! opening file");
        exit(1);
    }

    size_t n = 1;

    while (n > 0) {
        fread(src_code, sizeof(char), buf_sz, fd);

        //calculate the index/position of the last character written to the buffer
        for (size_t i=0; buf_sz > i; i++) {
            if (src_code[i] == 0) {
                n=i;
                break;
            }
        }

        if (n <= 0) {
            break;
        }

        size_t ntokens = tokenize(src_code, tokens[token_n], ctr, error);
        if (opts->print_tokens == 1) {
            printf("token stream: ");
            for (size_t p_i=0; ntokens > p_i; p_i++) {
                printf("[%s(%d,%d)] ", ptoken(tokens[p_i].type), (int)tokens[p_i].start, (int)tokens[p_i].end);
            }
            printf("\n\n");
        }

        int trap = 0;
        assemble_ast(src_code, tokens, ntokens, &root, &trap);
        memset(src_code, 0, buf_sz);
        
        n_completed = 0;
    }
    fclose(fd);
    
    if (synthesize(&root) == -1) {
        printf("synth failed");
    }
    if (opts->print_ast == 1) {    
        printf("\n\n");
        printf("----------------\n");
        printf("AST\n");
        printf("----------------\n");
        print_ast(&root);
    }

    if (opts->print_expr_tree == 1) {
        printf("\n\n");
        printf("----------------\n");
        printf("Expr Tree\n");
        printf("----------------\n");
        ExprStatement *temp;
        for (int i=0; root.length > i; i++) {
            temp=((ExprStatement *)root.statements[i]->internal_data);
            if (root.statements[i]->type == Expression) {
                printf("statement: %d\n", i+1);
                printf("length: %lu\n", expr_len(temp->expr));
                ptree(temp->expr);
                printf("----------------\n");
            }
        }
        
    }

    return 0;
}


int main(int argc, char* argv[]) {
    struct Opts opts;
    struct Token tokens[2048];
    struct BlockStatement root, current;

    if (argc == 1) {
        print_help(argv[0]);
        return 0;
    }

    init_opts(&opts);
    setup_opts(argc, argv, &opts);


    if(access(argv[argc-1], F_OK) == 0)
        return parse(argv[argc-1], &opts);
    

    printf("bad arguments\n");
    return 1;

}
