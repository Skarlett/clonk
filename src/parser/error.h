#ifndef _HEADER__PARSER_ERROR__
#define _HEADER__PARSER_ERROR__

typedef enum CompileTimeErrorT {
    CTE_UNDEFINED = 0,
    CTE_SYNTAX_EXPECTED = 1,
    CTE_BAD_STRING_LITERAL = 2,
    CTE_UNBALANCED_EXPR = 3,
    CTE_BAD_OPERAND = 4
} ErrorT;

struct CompileTimeError {
    enum CompileTimeErrorT kind;
    const char * file_path;
    
    union {
        struct {
            struct Token *from;
            struct Token *to;
            const char * msg;
        } syntax_error;
    } error;

};

#endif
