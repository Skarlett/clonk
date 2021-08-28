#ifndef _HEADER__LEXER__
#define _HEADER__LEXER__

#include "../CuTest.h"
#include <stdlib.h>

#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"

typedef enum Lexicon {
    UNDEFINED,

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
    
    // ??
    // ~!@#$%^&*+=-`
    SPECIAL_CHAR,

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

    //impl
    IMPL,
    
    // this is a 'pretend' token used 
    // internally by expression
    // and should never been seen in the token stream
    FNMASK,

    UNKNOWN
} Lexicon;

typedef struct Token {
    unsigned long start;
    unsigned long end;
    enum Lexicon token;
} Token;


enum Lexicon tokenize_char(char c);
int can_upgrade_token(enum Lexicon token);
int set_compound_token(enum Lexicon *compound_token, enum Lexicon token);

int is_operator_complex(enum Lexicon compound_token);
enum Lexicon invert_brace_type(enum Lexicon token);
enum Lexicon invert_operator_token(enum Lexicon compound_token);

const char * ptoken(enum Lexicon t);
int is_cmp_operator(enum Lexicon compound_token);
int is_bin_operator(enum Lexicon compound_token);
int is_data(enum Lexicon token);
int tokenize(char *line,  struct Token tokens[], size_t token_idx);
#endif
