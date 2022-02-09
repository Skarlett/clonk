#ifndef _HEADER__PARSER_ERROR__
#define _HEADER__PARSER_ERROR__
#include <stdbool.h>
#include <stdint.h>

enum Severity {
    /* annony you */
    Warning,

    /* Cannot compile document */
    Error,

    /* Crashes Compiler - default */
    Fatal
};


enum ErrorStage {
    ES_Unknown,
    ES_Lexer,
    ES_Parser,
};


enum ErrorT {
    ErrorUnknownT,
    ErrorParserT
};

/* struct ParseError { */
/*     const struct Token *span_start; */
/*     const struct Token *span_stop; */
/* }; */

struct UnexpectedToken {
    const struct Token *span_start;
    const struct Token *span_stop;
    const struct Token *suggestions;
};

struct Error {
    enum ErrorStage stage;
    enum Severity severity;
    enum ErrorT type;

    const char * file;
    const char * msg;

    union {
        //struct ParseError parser;
        
    } data;
};

void _mk_error(
    struct Error *err_out,
    void *err_in,
    enum ErrorT in_type,
    enum Severity severity,
    const char *msg,
    const char *file,
);

/**
 * @param err_in: Copies struct into err_out.data 
 */
#define mk_error(err_out, err_in, err_in_t, severity, msg) \

#endif
