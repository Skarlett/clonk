#ifndef _HEADER__LEXER__
#define _HEADER__LEXER__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "../../prelude.h"
#include "../error.h"

#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"

enum Lexicon {
    UNDEFINED = 0,
    
    NULLTOKEN,
    
    // Ignore token (whitespace, newline, carriage return)
    WHITESPACE,
    
    NEWLINE,

    // Ignore token (whitespace, newline, carriage return)
    COMMENT,

    // [
    BRACKET_OPEN,

    // ]
    BRACKET_CLOSE,

    // {
    BRACE_OPEN,
    
    // }
    BRACE_CLOSE,
    
    // (
    PARAM_OPEN,
    
    // )
    PARAM_CLOSE,

    // !
    NOT,

    // +
    ADD,
    
    // -
    SUB,

    // >
    GT,

    // <
    LT,

    // >=
    GTEQ,

    // <=
    LTEQ,

    // *
    MUL,

    // /
    DIV,

    // ^
    POW,

    // %
    MOD,

    // +=
    PLUSEQ,

    // -=
    MINUSEQ,
    
    // =
    EQUAL,

    // ==
    ISEQL,

    // !=
    ISNEQL,
    
    // &
    AMPER,

    // |
    PIPE,

    // &&
    AND,

    // ||
    OR,

    // "
    QUOTE,
    
    // ;
    SEMICOLON,

    // _
    UNDERSCORE,

    // .
    DOT,

    // #
    POUND,

    // :
    COLON,

    // @
    ATSYM,

    // a-zA-Z
    CHAR,

    // 0-9
    DIGIT,
    
    // ,
    COMMA,
    //********* START OF COMPLEX TOKENS ********
    // Complex tokens wont show up in the first round of lexer'ing
    // they're generated from combinations of tokens
    // "fn"
    //********* START OF COMPLEX LEXICONS ********

    // [NUM, ..] WHITESPACE|SEMICOLON   
    // 20392
    INTEGER,

    // [CHARACTER, ..] WHITESPACE|SEMICOLON
    // something
    WORD,

    // [QUOTE, ... QUOTE]
    // something
    STRING_LITERAL,

    // static
    STATIC,
    // const
    CONST,

    // return
    RETURN,
    
    // extern
    EXTERN,
    
    // as
    AS,
    
    // if
    IF,

    // else
    ELSE,

    // def
    FUNC_DEF,

    //import
    IMPORT,
    
    // end of file
    EOFT = 255,
    
    //impl
    IMPL,
    
    /*
        this is a 'pretend' token used 
        internally by expression
        and should never been seen in the token stream
    */

    MARKER,
};

/*
    Tokens reference symbols derived from the 
    source code given to the interpreter.
    the following fields correlate with the following,
    except when `type` is equal to FNMASK, or COMPOSITE.
    
    `start` correlates to the starting position the token inside the source code string.
    The position is directly indexable against its source (char *line).

    `end` correlates to the ending position of the token inside the source code string.
    The position is directly indexable against its source (char *line). 

    `type` defines what kind of token it is.

    In the case of `type` being of MARKER:
        `start` correlates to the starting position of the token instead the token array.
        The position is directly indexable against its source (tokens[]).

        `end` correlates to the ending position of the token instead the token array.
        The position is directly indexable against its source (tokens[]).
*/
struct Token {
    usize start;
    usize end;
    enum Lexicon type;
};

int8_t tokenize(
    char *line,
    struct Token tokens[],
    usize *token_ctr,
    usize token_sz,
    struct CompileTimeError *error
);

#endif
