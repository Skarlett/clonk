#ifndef _HEADER__AST__
#define _HEADER__AST__


int assemble_ast(
    char *line,
    struct Token tokens[],
    long unsigned ntokens,
    struct BlockStatement *block);
#endif