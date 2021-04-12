#ifndef HEADER_FILE
#define HEADER_FILE

#include <stdio.h>
#include <string.h>

#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"
/* ------------------------------------------ */
/*            lexer tokens                    */
/* ------------------------------------------ */

enum Lexicon {
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
};


const char * ptoken(enum Lexicon t) {
    if (t == INTEGER) return "integer";
    else if (t == WORD) return "word";
    else if (t == NULLTOKEN) return "nulltoken";
    else if (t == WHITESPACE) return "whitespace";
    else if (t == OPEN_BRACE) return "brack_open";
    else if (t == CLOSE_BRACK) return "brack_close";
    else if (t == PARAM_OPEN) return "param_open";
    else if (t == PARAM_CLOSE) return "param_close";
    else if (t == COMMA) return "comma";
    else if (t == DIGIT) return "digit";
    else if (t == QUOTE) return "quote";
    else if (t == EQUAL) return "eq";
    else if (t == ADD) return "add";
    else if (t == MUL) return "multiply";
    else if (t == DIV) return "divide";
    else if (t == GT) return "greater than";
    else if (t == LT) return "less than";
    else if (t == ISEQL) return "is eq";
    else if (t == GTEQ) return "greater than or eq";
    else if (t == LTEQ) return "less than or eq";
    else if (t == POW) return "exponent";
    else if (t == PLUSEQ) return "plus eq";
    else if (t == MINUSEQ) return "minus eq";
    else if (t == MOD) return "modolus";
    else if (t == SUB) return "sub";
    else if (t == SEMICOLON) return "semi-colon";
    else if (t == SPECIAL_CHAR) return "special_char";
    else if (t == CHAR) return "char";
    else if (t == STRING_LITERAL) return "str_literal";
    else if (t == UNKNOWN) return "unknown token";
    else if (t == AMPER) return "&";
    else if (t == PIPE) return "pipe";
    else if (t == AND) return "and";
    else if (t == OR) return "or";
    else return "PTOKEN_ERROR_UNKNOWN_TOKEN";
}


enum Lexicon tokenize_char(char c) {
    if (' ' == c
        || c == '\n'
        || c == '\r'
        || c == '\t' )
        return WHITESPACE;
    else if (c == '=')  return EQUAL;
    else if (c == '"')  return QUOTE;
    else if (c == '{')  return OPEN_BRACE;
    else if (c == '}')  return CLOSE_BRACK;
    else if (c == '(')  return PARAM_OPEN;
    else if (c == ')')  return PARAM_CLOSE;
    else if (c == ';')  return SEMICOLON;
    else if (c == ',')  return COMMA;
    else if (c == '+')  return ADD;
    else if (c == '-')  return SUB;
    else if (c == '*')  return MUL;
    else if (c == '^')  return POW;
    else if (c == '/')  return DIV;
    else if (c == '>')  return GT;
    else if (c == '<')  return LT;
    else if (c == '&')  return AMPER;
    else if (c == '|')  return PIPE;
    else if (c == '%')  return MOD;

    for (int i = 0; sizeof(DIGITS) > i; i++) {
        if (c == DIGITS[i])
            return DIGIT;
    }

    for (int i = 0; sizeof(ALPHABET) > i; i++) {
        if (c == ALPHABET[i])
            return CHAR;
    }

    return UNKNOWN;
}

struct Token {
    unsigned long start;
    unsigned long end;
    enum Lexicon token;
};


int is_compound_token(enum Lexicon token) {
    return (
        token == DIGIT
        || token == CHAR
        || token == QUOTE
        || token == EQUAL
        || token == GT
        || token == LT
        || token == ADD
        || token == SUB
        || token == AMPER
        || token == PIPE
    );
}

int set_compound_token(enum Lexicon token, enum Lexicon *compound_token) {
    if (token == DIGIT) 
        *compound_token = INTEGER;
    
    else if (token == CHAR)
        *compound_token = WORD;
    
    else if (token == QUOTE)
        *compound_token = STRING_LITERAL;
    
    else if (token == EQUAL)
        *compound_token = ISEQL;
    
    // maybe these are bit shift operations
    // wel'll find out later
    else if (token == GT)
        *compound_token = GTEQ;
    
    else if (token == LT)
        *compound_token = LTEQ;
    // ----

    else if (token == ADD)
        *compound_token = PLUSEQ;

    else if (token == SUB)
        *compound_token = MINUSEQ;
    
    else if (token == AMPER)
        *compound_token = AND;

    else if (token == PIPE)
        *compound_token = OR;

    else
        return -1;
    
    return 0;
}

int continue_compound(enum Lexicon token, enum Lexicon compound_token) {
    return (
        compound_token == INTEGER && token == DIGIT 
        || compound_token == WORD && token == CHAR
        || compound_token == STRING_LITERAL && token != QUOTE
        || compound_token == ISEQL && token == EQUAL
        || compound_token == GTEQ && token == EQUAL
        || compound_token == LTEQ && token == EQUAL
        || compound_token == AND && token == AMPER
        || compound_token == OR && token == PIPE 
        || compound_token == PLUSEQ && token == EQUAL
        || compound_token == MINUSEQ && token == EQUAL
    );
}

int is_operator_compound(enum Lexicon compound_token) {
    return (
        compound_token == ISEQL 
        || compound_token == GTEQ 
        || compound_token == LTEQ
        || compound_token == AND
        || compound_token == OR
        || compound_token == MINUSEQ
        || compound_token == PLUSEQ
    );
}

enum Lexicon invert_operator_token(enum Lexicon compound_token) {
    if (compound_token == ISEQL) return EQUAL;
    else if (compound_token == GTEQ) return GT;
    else if (compound_token == LTEQ) return LT;
    else if (compound_token == AND) return AMPER;
    else if (compound_token == OR) return PIPE;
    else if (compound_token == MINUSEQ) return SUB;
    else if (compound_token == PLUSEQ) return ADD;
    else return NULLTOKEN;
}

int tokenize(char *line,  struct Token tokens[], int token_idx) {
    enum Lexicon compound_token = NULLTOKEN;
    unsigned long compound_start = 0;
    unsigned long ctr = 0;
    int original = token_idx;

    enum Lexicon lexed;

    for (unsigned long i=0; strlen(line) > i; i++) {
        if (line[i] == 0) continue;

        lexed = tokenize_char(line[i]);
                
        if (compound_token == NULLTOKEN) {
            if (is_compound_token(lexed))
            {
                set_compound_token(lexed, &compound_token);             
                compound_start = i;
                continue;
            }
        }

        else if (continue_compound(lexed, compound_token))
            continue;
        
        // a token that holds multiple unitary tokens in it, 
        // has broken sequence
        else if (compound_token != NULLTOKEN) {
            
            struct Token token = {
                .start = compound_start,
                .end = i,
                .token = compound_token
            };

            // check if its an operator, and that its lenth is 2
            if (is_operator_compound(compound_token) && i-compound_start != 2 ) {
                printf("compound start-i: %d\n", i-compound_start);
                token.start = i-1;
                token.end = i-1;
                token.token = invert_operator_token(compound_token);
            }
            
            compound_start = 0;
            tokens[token_idx+ctr] = token;
            ctr += 1;

            compound_token = NULLTOKEN;
            // skip the extra quote token
            if (compound_token == STRING_LITERAL) {
                continue;
            }
        }

        if (lexed != WHITESPACE) {
            struct Token token = {
                .start = i,
                .end = i,
                .token = lexed
            };

            compound_token = NULLTOKEN;
            compound_start = 0;
            tokens[token_idx+ctr] = token;
            ctr += 1;
        }
    }
    
    if (compound_token != NULLTOKEN) {
        struct Token token = {
            .start = compound_start,
            .end = strlen(line),
            .token = compound_token
        };
        tokens[token_idx+ctr] = token;
        ctr += 1;
    }

    return ctr;
}

#endif