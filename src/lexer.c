#include "lexer.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------ */
/*            lexer tokens                    */
/* ------------------------------------------ */

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


int is_complex_token(enum Lexicon token) {
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

int set_complex_token(enum Lexicon token, enum Lexicon *complex_token) {
    if (token == DIGIT) 
        *complex_token = INTEGER;
    
    else if (token == CHAR)
        *complex_token = WORD;
    
    else if (token == QUOTE)
        *complex_token = STRING_LITERAL;
    
    else if (token == EQUAL)
        *complex_token = ISEQL;
    
    // maybe these are bit shift operations
    // wel'll find out later
    else if (token == GT)
        *complex_token = GTEQ;
    
    else if (token == LT)
        *complex_token = LTEQ;
    // ----

    else if (token == ADD)
        *complex_token = PLUSEQ;

    else if (token == SUB)
        *complex_token = MINUSEQ;
    
    else if (token == AMPER)
        *complex_token = AND;

    else if (token == PIPE)
        *complex_token = OR;

    else
        return -1;
    
    return 0;
}

int continue_complex(enum Lexicon token, enum Lexicon complex_token) {
    return (
        complex_token == INTEGER && token == DIGIT 
        || complex_token == WORD && token == CHAR
        || complex_token == STRING_LITERAL && token != QUOTE
        || complex_token == ISEQL && token == EQUAL
        || complex_token == GTEQ && token == EQUAL
        || complex_token == LTEQ && token == EQUAL
        || complex_token == AND && token == AMPER
        || complex_token == OR && token == PIPE 
        || complex_token == PLUSEQ && token == EQUAL
        || complex_token == MINUSEQ && token == EQUAL
    );
}

int is_operator_complex(enum Lexicon complex_token) {
    return (
        complex_token == ISEQL 
        || complex_token == GTEQ 
        || complex_token == LTEQ
        || complex_token == AND
        || complex_token == OR
        || complex_token == MINUSEQ
        || complex_token == PLUSEQ
        || complex_token == GT
        || complex_token == LT
    );
}

enum Lexicon invert_operator_token(enum Lexicon complex_token) {
    if (complex_token == ISEQL) return EQUAL;
    else if (complex_token == GTEQ) return GT;
    else if (complex_token == LTEQ) return LT;
    else if (complex_token == AND) return AMPER;
    else if (complex_token == OR) return PIPE;
    else if (complex_token == MINUSEQ) return SUB;
    else if (complex_token == PLUSEQ) return ADD;
    else return NULLTOKEN;
}

// ---
// pub
int is_cmp_operator(enum Lexicon complex_token) {
    return (
        complex_token == ISEQL 
        || complex_token == GTEQ 
        || complex_token == LTEQ
        || complex_token == AND
        || complex_token == OR
    );
}

int is_bin_operator(enum Lexicon complex_token) {
    return (complex_token == ISEQL 
        || complex_token == GTEQ 
        || complex_token == LTEQ
        || complex_token == AND
        || complex_token == OR
        || complex_token == MINUSEQ
        || complex_token == PLUSEQ
        || complex_token == GT
        || complex_token == LT);
}


int is_data(enum Lexicon token) {
    return (token == WORD 
        || token == INTEGER 
        || token == STRING_LITERAL
    );
}

int tokenize(char *line,  struct Token tokens[], size_t token_idx) {
    enum Lexicon complex_token = NULLTOKEN;
    unsigned long complex_start = 0;
    unsigned long ctr = 0;
    int original = token_idx;

    enum Lexicon lexed;

    for (unsigned long i=0; strlen(line) > i; i++) {
        if (line[i] == 0) continue;

        lexed = tokenize_char(line[i]);
                
        if (complex_token == NULLTOKEN) {
            if (is_complex_token(lexed))
            {
                set_complex_token(lexed, &complex_token);             
                complex_start = i;
                continue;
            }
        }

        else if (continue_complex(lexed, complex_token))
            continue;
        
        // a token that holds multiple unitary tokens in it, 
        // has broken sequence
        else if (complex_token != NULLTOKEN) {
            
            struct Token token = {
                .start = complex_start,
                .end = i,
                .token = complex_token
            };

            // check if its an operator, and that its lenth is 2
            if (is_operator_complex(complex_token) && i-complex_start != 2 ) {
                printf("complex start-i: %d\n", (int)(i-complex_start));
                token.start = i-1;
                token.end = i-1;
                token.token = invert_operator_token(complex_token);
            }
            
            complex_start = 0;
            tokens[token_idx+ctr] = token;
            ctr += 1;

            complex_token = NULLTOKEN;
            // skip the extra quote token
            if (complex_token == STRING_LITERAL) {
                continue;
            }
        }

        if (lexed != WHITESPACE) {
            struct Token token = {
                .start = i,
                .end = i,
                .token = lexed
            };

            complex_token = NULLTOKEN;
            complex_start = 0;
            tokens[token_idx+ctr] = token;
            ctr += 1;
        }
    }
    
    if (complex_token != NULLTOKEN) {
        struct Token token = {
            .start = complex_start,
            .end = strlen(line),
            .token = complex_token
        };
        tokens[token_idx+ctr] = token;
        ctr += 1;
    }

    return ctr;
}
