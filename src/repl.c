// CLONK interpreter
// my very own 1990s retro built interpreter
#include "clonk.h"
#include "lexer.h"
#include "parser.h"

#include <stdio.h>
#include <stdint.h>

#define BANNER                                                  \
    "    {__    {__                   {__\r\n"                  \
    " {__   {__ {__                   {__\r\n"                  \
    "{__        {__   {__     {__ {__ {__  {__\r\n"             \
    "{__        {__ {__  {__  {__  {__{__ {__\r\n"              \
    "{__        {__{__    {__ {__  {__{_{__\r\n"                \
    " {__   {__ {__ {__  {__  {__  {__{__ {__\r\n"              \
    "   {____  {___   {__    {___  {__{__  {__\r\n"             \
    "License: clonk-license.txt \n"                             \
    "bugs report: https://github.com/skarlett/clonk/issues\n"   \
    "Copyright 2021-2022 - version: "

#define HELP_TEXT \
    "\t -h | brings up this help menu\n" \
    "\t -V | print version\n" \
    "\t -x --examine <tokenizer|parser|ast>,... \n" \


void print_help(char *name) {
    printf("usage: %s [opts] [file]\nversion: %s\n\n", name, ONK_VERSION);
    printf("%s\n", HELP_TEXT);
}

struct Opts {
    uint8_t print_ast;
    uint8_t print_parser;
    uint8_t print_tokens;
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
            printf("%s\n", ONK_VERSION);
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


int main(int argc, char* argv[]) {
    struct Opts opts;
    uint16_t i = 0;
    struct onk_lexer_input_t input;
    struct onk_lexer_output_t token_output;

    struct onk_parser_input_t parser_input;
    struct onk_parser_output_t parser_output;

    input.src_code = "4 + 4";
    
    if (argc == 1) {
        print_help(argv[0]);
        return 0;
    }

    init_opts(&opts);
    setup_opts(argc, argv, &opts);
    
    if(onk_tokenize(&input, &token_output) == -1)
      return -1;

    for (i=0; token_output.tokens.len > i; i++)
    {
        printf("[%s] ", onk_ptoken(((struct onk_token_t *)token_output.tokens.base)[i].type));
    }
    puts("\n");

    onk_parser_input_from_lexer_output(&token_output, &parser_input, false);

    if(onk_parse(&parser_input, &parser_output) == -1)
      return -2;

    for (i=0; parser_output.postfix.len > i; i++)
    {
        printf("[%s] ", onk_ptoken(((struct onk_token_t **)parser_output.postfix.base)[i]->type));
    }
    puts("\n");
    
    /* if(access(argv[argc-1], F_OK) == 0) */
    /*     return parse(argv[argc-1], &opts); */
    /* printf("bad arguments\n"); */
    return 1;

}
