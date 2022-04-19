// CLONK interpreter
// my very own 1990s retro built interpreter
#include "clonk.h"
#include <stdint.h>

#define BANNER \
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


int main(int argc, char* argv[]) {
    struct Opts opts;
    struct onk_token_t * tokens;

    if (argc == 1) {
        print_help(argv[0]);
        return 0;
    }

    init_opts(&opts);
    setup_opts(argc, argv, &opts);



    /* if(access(argv[argc-1], F_OK) == 0) */
    /*     return parse(argv[argc-1], &opts); */


    /* printf("bad arguments\n"); */
    return 1;

}
