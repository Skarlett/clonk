#ifndef _HEADER__PARSER_ERROR__
#define _HEADER__PARSER_ERROR__

enum ErrorT {
    Warning,
    Error,
    Fatal
};

enum ComponentSource {
    Lexer,
    ExprParser,
    AST,
};


struct CompileTimeError {
    struct Token *base;
    enum ErrorT type;
    const char * file;
    bool free_msg;
    const char * msg;
};

#endif
