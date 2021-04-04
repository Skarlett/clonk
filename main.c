#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0
#define EXTENSION "ka"

#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"

enum Error {
    OK = 0,
    ExpectToken,
};

enum Lexicon {
    NULLTOKEN,
    
    // Ignore token (whitespace, newline, carriage return)
    WHITESPACE,

    // {
    OPEN_BRACK,
    
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

    // =
    EQUAL,

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

    UNKNOWN,
};


const char * ptoken(enum Lexicon t) {
    if (t == INTEGER) return "integer";
    else if (t == WORD) return "word";
    else if (t == NULLTOKEN) return "nulltoken";
    else if (t == WHITESPACE) return "whitespace";
    else if (t == ADD) return "add";
    else if (t == OPEN_BRACK) return "brack_open";
    else if (t == CLOSE_BRACK) return "brack_close";
    else if (t == PARAM_OPEN) return "param_open";
    else if (t == PARAM_CLOSE) return "param_close";
    else if (t == COMMA) return "comma";
    else if (t == DIGIT) return "digit";
    else if (t == QUOTE) return "quote";
    else if (t == EQUAL) return "eq";
    else if (t == SUB) return "sub";
    else if (t == SEMICOLON) return "semi-colon";
    else if (t == UNKNOWN) return "unknown";
    else if (t == SPECIAL_CHAR) return "special_char";
    else if (t == CHAR) return "char";
    else return "PTOKEN_ERROR_UNKNOWN_TOKEN";
}

enum Lexicon tokenize_char(char c) {
    if (' ' == c
        || c == '\n'
        || c == '\r'
        || c == '\t' )
        return WHITESPACE;
    else if (c == '+')  return ADD;
    else if (c == '-')  return SUB;
    else if (c == '=')  return EQUAL;
    else if (c == '"')  return QUOTE;
    else if (c == '{')  return OPEN_BRACK;
    else if (c == '}')  return CLOSE_BRACK;
    else if (c == '(')  return PARAM_OPEN;
    else if (c == ')')  return PARAM_CLOSE;
    else if (c == ';')  return SEMICOLON;
    else if (c == ',')  return COMMA;

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
    long int start;
    long int end;
    enum Lexicon token;
};

int tokenize(char *buf,  enum Lexicon *tokens) {
    printf("%s", buf);
    return 0;
}

int parse(char *filepath) {
    int buf_sz = 2048;
    char line[buf_sz];
    
    enum Lexicon complex_token = NULLTOKEN;
    long int complex_start = 0;

    struct Token tokens[buf_sz];
    int token_idx = 0;
        
    FILE *fd;

    memset(line, 0, buf_sz);
    memset(tokens, 0, sizeof(struct Token)*buf_sz);
    
    if ((fd = fopen(filepath, "r")) == NULL) {
        printf("Error! opening file");
        exit(1);
    }

    int n = 1;

    while (n > 0) {
        fgets(line, buf_sz, fd);

        // calculate the index/position of the last character written to the buffer
        for (long int i=0; buf_sz > i; i++) {
            if (line[i] == 0) {
                n=i;
                break;
            }
        }

        for (int i=0; sizeof(line) > i; i++) {
            if (line[i] == 0)
                break;
            
            enum Lexicon lexed = tokenize_char(line[i]);
            if (lexed == WHITESPACE) continue;

            printf("%s('%c') [%s]\n", ptoken(lexed), line[i], ptoken(complex_token));
            
            if (complex_token == NULLTOKEN) {
                if (lexed == DIGIT || lexed == CHAR) 
                {
                    if (lexed == DIGIT) 
                        complex_token = INTEGER;
                    else
                        complex_token = WORD;
                    
                }
                complex_start = i;
                continue;
            }
            else if (complex_token == INTEGER && lexed == DIGIT 
                    || complex_token == WORD && lexed == CHAR)
                continue;
            
            else if (complex_token == INTEGER || complex_token == WORD) {
                printf("storing_complex: %s, (%d..%d)\n", ptoken(complex_token), complex_start, i);

                struct Token token = {
                    .start = complex_start,
                    .end = i,
                    .token = complex_token
                };

                tokens[token_idx] = token;
                complex_token = NULLTOKEN;
                complex_start = 0;
                token_idx += 1;
            }
           
            struct Token token = {
                .start = complex_start,
                .end = i,
                .token = lexed
             };
                
            tokens[token_idx] = token;
            token_idx += 1;
            
        }
        memset(line, 0, buf_sz);
    }
    fclose(fd);
    printf("tokens: %d\n", token_idx);
    for (int i=0; token_idx > i; i++) {
        printf("[%s] ", ptoken(tokens[i].token));
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc > 1) {
        parse(argv[1]);
        return 0;
    }
    else {
        printf("%s [file.%s]\n", argv[0], EXTENSION);
        return 1;
    }
    
}

int strcontains(char *buf, char *pattern) {
    int window_sz = sizeof(pattern);
    char *slice;

    for (int win_idx=0; sizeof(buf) > win_idx; win_idx++) {
        slice = buf+win_idx;
        for (int pat_idx=0; window_sz > pat_idx; pat_idx++) {
            if (pattern[pat_idx] != slice[pat_idx]) {
                break;
            }
            else { if (pat_idx == window_sz) { return 0; } }
        }
    }

    return -1;
}

int strstartswith(char *buf, char *pattern) {
    for (int i=0; sizeof(pattern) > i; i++) {
        if (buf[i] != pattern[i]) {
            return -1;
        }
    }

    return 0;
}