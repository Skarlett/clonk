#ifndef _HEADER__PARSER_ERROR__
#define _HEADER__PARSER_ERROR__

#include "lexer/lexer.h"

typedef enum CompileTimeErrorT {
    CTE_UNDEFINED = 0,
    CTE_SYNTAX_EXPECTED = 1,
    CTE_BAD_STRING_LITERAL = 2,
    CTE_UNBALANCED_EXPR = 3,
    CTE_BAD_OPERAND = 4
} ErrorT;

static char *error_msg[] = {
    0,
    
    "syntax error: expected token '%s', got '%s'",
    
    "syntax error: missing end quote",
    
    "syntax error: missing closing '%s'",
    
    "syntax error: expected an expression after '%s'",

    0
};

struct CompileTimeError {
    enum CompileTimeErrorT kind;
    char * file_path;
    
    union {
        struct {
            struct Token *from;
            struct Token *to;
            char * msg;
        } syntax_error;

    } error;

};

#endif
