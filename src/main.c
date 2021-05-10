// CLONK interpreter
// my very own 1990s retro built interpreter
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "parser/lexer.h"
#include "parser/expr.h"
#include "parser/ast.h"
#include "productions/syn.h"
#include "common.h"


#define HELP_TEXT \
    "\t -h | brings up this help menu\n" \
    "\t -V | print version\n" \
    "\t -c | runs a string\n" \
    "\t -a | prints ast\n" \
    "\t -l | prints token stream\n"
   

void print_help(char *name) {
    printf("usage: %s [file]\nversion: %s\n\n", name, VERSION);
    printf("%s\n", HELP_TEXT);
}

// /* ------------------------------------------ */
// /*            Func call                       */
// /* ------------------------------------------ */
// enum Action {
//     NullCommand,
//     Repl,
//     RunFile,
//     RunString,
//     PrintTokenStream,
//     PrintAST,
//     //HelpMenu,
//     //PrintVersion,
// };


enum RunType {
    RunTypeUndefined,
    CommandString,
    FileInput
};

struct Opts {
    int print_ast;
    int print_tokens;
};

void init_opts(struct Opts *opts) {
    opts->print_ast=0;
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

    char line[buf_sz];
    memset(line, 0, buf_sz);
    
    if ((fd = fopen(fp, "r")) == NULL) {
        perror("Error! opening file");
        exit(1);
    }

    size_t n = 1;

    while (n > 0) {
        fread(line, sizeof(char), buf_sz, fd);

        //calculate the index/position of the last character written to the buffer
        for (size_t i=0; buf_sz > i; i++) {
            if (line[i] == 0) {
                n=i;
                break;
            }
        }

        if (n <= 0) {
            break;
        }

        size_t ntokens = tokenize(line, tokens, token_n);
        if (opts->print_tokens == 1) {
            printf("token stream: ");
            for (size_t p_i=0; ntokens > p_i; p_i++) {
                printf("[%s(%d,%d)] ", ptoken(tokens[p_i].token), (int)tokens[p_i].start, (int)tokens[p_i].end);
            }
            printf("\n\n");
        }

        int trap = 0;
        assemble_ast(line, tokens, ntokens, &root, &trap);
        memset(line, 0, buf_sz);
        
        n_completed = 0;
    }
    fclose(fd);    
    synthesize(&root);
    if (opts->print_ast == 1) {    
        printf("\n\n");
        printf("----------------\n");
        printf("AST\n");
        printf("----------------\n");
        print_ast(&root);
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
