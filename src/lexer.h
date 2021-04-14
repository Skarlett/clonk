#ifndef _HEADER__LEXER__
#define _HEADER__LEXER__

#include <stdlib.h>
#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"


typedef enum Lexicon {
    NULLTOKEN,
    
    // Ignore token (whitespace, newline, carriage return)
    WHITESPACE,

    // {
    OPEN_BRACE,
    
    // }
    CLOSE_BRACK,
    
    // (
    PARAM_OPEN,
    
    // )
    PARAM_CLOSE,

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

    // a-zA-Z
    CHAR,

    // 0-9
    DIGIT,
  
    // ~!@#$%^&*_+=-`
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

    // [CHARACTER, ..] WHITESAPCE|SEMICOLON
    // something
    WORD,

    // [QUOTE, ... QUOTE]
    // something
    STRING_LITERAL,


    UNKNOWN,
} Lexicon;

typedef struct Token {
    unsigned long start;
    unsigned long end;
    enum Lexicon token;
} Token;


enum Lexicon tokenize_char(char c);
int is_complex_token(enum Lexicon token);
int set_complex_token(enum Lexicon token, enum Lexicon *compound_token);
int continue_complex(enum Lexicon token, enum Lexicon compound_token);
int is_operator_compound(enum Lexicon compound_token);
enum Lexicon invert_operator_token(enum Lexicon compound_token);


const char * ptoken(enum Lexicon t);
int is_cmp_operator(enum Lexicon compound_token);
int is_bin_operator(enum Lexicon compound_token);
int is_data(enum Lexicon token);
int tokenize(char *line,  struct Token tokens[], size_t token_idx);
#endif