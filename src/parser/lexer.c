#include "lexer.h"
#include <stdio.h>
#include <string.h>

/* ------------------------------------------ */
/*            lexer tokens                    */
/* ------------------------------------------ */

const char * ptoken(enum Lexicon t) {
    switch (t) {
        case INTEGER: return "integer";
        case WORD: return "word";
        case NULLTOKEN: return "nulltoken";
        case WHITESPACE: return "whitespace";
        case OPEN_BRACE: return "brace_open";
        case CLOSE_BRACE: return "brace_close";
        case PARAM_OPEN: return "param_open";
        case PARAM_CLOSE: return "param_close";
        case COMMA: return "comma";
        case DIGIT: return "digit";
        case QUOTE: return "quote";
        case EQUAL: return "eq";
        case ADD: return "add";
        case MUL: return "multiply";
        case DIV: return "divide";
        case GT: return "greater than";
        case LT: return "less than";
        case ISEQL: return "is eq";
        case GTEQ: return "greater than or eq";
        case LTEQ: return "less than or eq";
        case POW: return "exponent";
        case PLUSEQ: return "plus eq";
        case MINUSEQ: return "minus eq";
        case MOD: return "modolus";
        case SUB: return "sub";
        case SEMICOLON: return "semi-colon";
        case SPECIAL_CHAR: return "special_char";
        case CHAR: return "char";
        case STRING_LITERAL: return "str_literal";
        case UNKNOWN: return "unknown token";
        case AMPER: return "&";
        case PIPE: return "pipe";
        case AND: return "and";
        case OR: return "or";
        case UNDERSCORE: return "underscore";
        case NOT: return "exclaimation";
        default: return "PTOKEN_ERROR_UNKNOWN_TOKEN";
    };
}


enum Lexicon tokenize_char(char c) {

    switch (c) {
        case ' ': return WHITESPACE;
        case '\n': return WHITESPACE;
        case '\t': return WHITESPACE;
        case '\r': return WHITESPACE;
        
        case '/':  return DIV;
        case '=':  return EQUAL;
        case '"':  return QUOTE;
        case '{':  return OPEN_BRACE;
        case '}':  return CLOSE_BRACE;
        case '(':  return PARAM_OPEN;
        case ')':  return PARAM_CLOSE;
        case ';':  return SEMICOLON;
        case ',':  return COMMA;
        case '+':  return ADD;
        case '-':  return SUB;
        case '*':  return MUL;
        case '^':  return POW;
        case '>':  return GT;
        case '<':  return LT;
        case '&':  return AMPER;
        case '|':  return PIPE;
        case '%':  return MOD;
        case '_':  return UNDERSCORE;
        case '!':  return NOT;
        default :  break;
    }

    
    for (int d_i = 0; sizeof(DIGITS) > d_i; d_i++) {
        if (c == DIGITS[d_i])
            return DIGIT;
    }

    for (int a_i = 0; sizeof(ALPHABET) > a_i; a_i++) {
        if (c == ALPHABET[a_i])
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
        || token == NOT
        || token == GT
        || token == LT
        || token == ADD
        || token == SUB
        || token == AMPER
        || token == PIPE
    );
}

int set_complex_token(enum Lexicon token, enum Lexicon *complex_token) {
    switch (token) {
        case DIGIT: 
            *complex_token = INTEGER;
            break;
        
        case CHAR:
            *complex_token = WORD;
            break;

        case QUOTE:
            *complex_token = STRING_LITERAL;
            break;

        case EQUAL:
            *complex_token = ISEQL;
            break;

        case GT:
            *complex_token = GTEQ;
            break;
        
        case LT:
            *complex_token = LTEQ;
            break;
        
        case ADD:
            *complex_token = PLUSEQ;
            break;
        
        case SUB:
            *complex_token = MINUSEQ;
            break;
        
        case AMPER:
            *complex_token = AND;
            break;
        
        case NOT:
            *complex_token = ISNEQL;
            break;
        

        case PIPE:
            *complex_token = OR;
            break;

        default: return -1;
    }
    
    return 0;
}

int continue_complex(enum Lexicon token, enum Lexicon complex_token) {
    return (
        complex_token == INTEGER && token == DIGIT 
        || complex_token == WORD && token == CHAR
        || complex_token == WORD && token == UNDERSCORE
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
    switch (complex_token) {
        case ISEQL: return EQUAL;
        case GTEQ: return GT;
        case LTEQ: return LT;
        case AND: return AMPER;
        case OR: return PIPE;
        case MINUSEQ: return SUB;
        case PLUSEQ: return ADD;
        default: return NULLTOKEN;
    }
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
        || complex_token == LT
        || complex_token == ADD
        || complex_token == SUB
        || complex_token == MUL
        || complex_token == POW
        || complex_token == NOT
        || complex_token == MOD
        || complex_token == DIV
    );
}


int is_keyword(char *line, Token t) {
    char *keywords[4] = {"if", "elif", "return", "else"};
    for (int i=0; 4 > i; i++) {
        if (strncmp(t.start+line, keywords[i], strlen(keywords[i])) == 0)
            return 1;
    }
    return 0;
}


int is_symbol(char *line, Token token) {
    return (token.token == WORD
        && !is_keyword(line, token)
    );
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
        
        // a token that holds multiple tokens in it, 
        // has broken sequence
        else if (complex_token != NULLTOKEN) {
            
            struct Token token = {
                .start = complex_start,
                .end = i,
                .token = complex_token
            };

            // check if its an operator, and that its lenth is 2
            if (is_operator_complex(complex_token) && i-complex_start != 2 ) {
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
