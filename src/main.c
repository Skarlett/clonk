// CLONK interpreter
// my very own 1990s retro built interpreter
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "core/lexer.h"
#include "core/expr.h"
#include "core/ast.h"
#include "core/common.h"


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
    enum RunType type;
    char *run;
};

void init_opts(struct Opts *opts) {
    opts->print_ast=0;
    opts->print_tokens=0;
    opts->type=RunTypeUndefined;
    opts->run = NULL;
}

int access_chk(int argc, int i, char *argv[], struct Opts *opts) {
    if (argc > i) {
        opts->run = argv[i+1];
        return 0;
    }
    else {
        printf("expected token to follow");
        exit(1);
    }
    return -1;
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

        else if (strcmp(argv[i], "-l") == 0) {
            opts->print_tokens=1;
        }

        else if (strcmp(argv[i], "-a") == 0) {
            opts->print_tokens=1;
        }

        else if (strcmp(argv[i], "-c") == 0) {
            access_chk(argc, i, argv, opts);
            skip = 1;
        }

        else if(access(argv[i], F_OK) == 0) {
            access_chk(argc, i, argv, opts);
            skip = 1;
        }

        else {
            printf("unknown option '%s'\n", argv[i]);
            exit(1);
        }
    }
}



int parse(struct Opts *opts) {
    FILE *fd;
    
    struct BlockStatement root;
    init_block(&root, STMT_CAPACITY*2);


    struct Token tokens[2048];

    size_t n_completed = 0;
    size_t buf_sz = 2048;
    size_t token_n = 0;

    char line[buf_sz];
    memset(line, 0, buf_sz);
    
    if ((fd = fopen(opts->run, "r")) == NULL) {
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
        printf("token stream: ");
        for (size_t i=0; ntokens > i; i++) {
            printf("[%s(%d,%d)] ", ptoken(tokens[i].token), (int)tokens[i].start, (int)tokens[i].end);
        }
        printf("\n\n");
        
        assemble_ast(line, tokens, ntokens, &root);
        memset(line, 0, buf_sz);
        
        n_completed = 0;
    }
    fclose(fd);    
    
    
    printf("\n\n");
    printf("----------------\n");
    printf("AST\n");
    printf("----------------\n");
    print_ast(&root);
    return 0;
}


int main(int argc, char *argv[]) {
    struct Opts opts;
    struct Token tokens[2048];
    struct BlockStatement root, current;

    if (argc == 1) {
        print_help(argv[0]);
        return 0;
    }

    init_opts(&opts);
    setup_opts(argc, argv, &opts);

    if (opts.run==0) {
        printf("bad arguments\n");
        return 1;
    }

    return parse(&opts);
}
