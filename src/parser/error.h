#ifndef _HEADER__PARSER_ERROR__
#define _HEADER__PARSER_ERROR__


enum ErrorT {
    Warning,
    Error,
    Fatal
};

struct CompileTimeError {
    struct Token *from;
    char * file;
    char * msg;
};

struct Error {
    struct CompileTimeError ;
};

#endif
