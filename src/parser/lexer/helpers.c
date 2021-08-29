#include "lexer.h"

const char * ptoken(enum Lexicon t) {
    switch (t) {
        case INTEGER: return "integer";
        case WORD: return "word";
        case CHAR: return "char";
        case NULLTOKEN: return "nulltoken";
        case WHITESPACE: return "whitespace";
        case NEWLINE: return "newline";
        case BRACE_OPEN: return "brace_open";
        case BRACE_CLOSE: return "brace_close";
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
        case ISNEQL: return "not eq";
        case GTEQ: return "greater than or eq";
        case LTEQ: return "less than or eq";
        case POW: return "exponent";
        case PLUSEQ: return "plus eq";
        case MINUSEQ: return "minus eq";
        case MOD: return "modolus";
        case SUB: return "sub";
        case COLON: return "colon";
        case SEMICOLON: return "semi-colon";
        case SPECIAL_CHAR: return "special_char";
        case STRING_LITERAL: return "str_literal";
        case UNKNOWN: return "unknown token";
        case AMPER: return "&";
        case PIPE: return "pipe";
        case AND: return "and";
        case OR: return "or";
        case UNDERSCORE: return "underscore";
        case NOT: return "not";
        case POUND: return "pound";
        case STATIC: return "'static'";
        case CONST: return "'const'";
        case IF: return "'if";
        case ELSE: return "'else'";
        case IMPL: return "'impl'";
        case FUNC_DEF: return "'def'";
        case FNMASK: return "fn_call(..)";
        case RETURN: return "'return'";
        case AS: return "'as'";
        case ATSYM: return "@";
        case IMPORT: return "'import'";
        case EXTERN: return "'extern'";
        case COMMENT: return "comment";

        default: return "PTOKEN_ERROR_UNKNOWN_TOKEN";
    };
}

enum Lexicon invert_brace_type(enum Lexicon token) {
    switch (token) {
        case PARAM_OPEN: return PARAM_CLOSE;
        case PARAM_CLOSE: return PARAM_OPEN;
        case BRACE_OPEN: return BRACE_CLOSE;
        case BRACE_CLOSE: return BRACE_OPEN;
        case BRACKET_CLOSE: return BRACKET_OPEN;
        case BRACKET_OPEN: return BRACKET_CLOSE;
        default: return UNKNOWN;
    }
}

int inner_balance(enum Lexicon tokens[], size_t *tokens_ctr, enum Lexicon current) {
    enum Lexicon inverted;

    if (is_close_brace(current)) {
        inverted = invert_brace_type(current);

        if (*tokens_ctr <= 0 || inverted == -1)
            return -1;
        
        if (tokens[(*tokens_ctr)-1] == inverted)
            (*tokens_ctr)--;
        else
            return -1;
    }

    else if (is_open_brace(current)) {
        *tokens_ctr += 1;
        tokens[(*tokens_ctr)-1] = current;
        return 1;
    }

    return 0;
}

/* 
    function determines if an expression is unbalanced.
    an expression can be unbalanced 
    if there is a nonmatching `[` / `(` / `{` character
*/
int is_balanced(struct Token tokens[], size_t ntokens) {
    enum Lexicon braces[512];
    size_t braces_ctr = 0;
    
    for (int i=0; ntokens > i; i++){
        if (inner_balance(braces, &braces_ctr, tokens[i].token) == -1)
            return 0;
    }
    return braces_ctr == 0;
}


/* 
    function determines if an expression is unbalanced.
    an expression can be unbalanced 
    if there is a nonmatching `[` / `(` / `{` character
*/
int is_balanced_by_ref(struct Token *tokens[], size_t ntokens) {
    enum Lexicon braces[512];
    size_t braces_ctr = 0;
    
    for (int i=0; ntokens > i; i++){
        if (inner_balance(braces, &braces_ctr, tokens[i]->token) == -1)
            return 0;
    }
    return braces_ctr == 0;
}


int is_close_brace(enum Lexicon token) {
    return (token == PARAM_CLOSE 
    || token == BRACE_CLOSE  
    || token == BRACKET_CLOSE);
}

int is_open_brace(enum Lexicon token) {
    return (token == PARAM_OPEN 
    || token == BRACE_OPEN  
    || token == BRACKET_OPEN);
}

int is_keyword(enum Lexicon token) {
    static enum Lexicon keywords[10] = {
        STATIC, CONST, RETURN, EXTERN, 
        AS, IF, ELSE, FUNC_DEF, IMPORT, IMPL
    };
    
    for (int i=0; 10 > i; i++) {
        if (token == keywords[i])
            return 1;
    }
    
    return 0;
}

int is_data(enum Lexicon token) {
    return (token == WORD
        || token == INTEGER 
        || token == STRING_LITERAL
    );
}

int is_cmp_operator(enum Lexicon compound_token) {
    return (
        compound_token == ISEQL
        || compound_token == ISNEQL  
        || compound_token == GTEQ 
        || compound_token == LTEQ
        || compound_token == AND
        || compound_token == OR
    );
}

int is_assignment_operator(enum Lexicon compound_token) {
    return (
        compound_token == EQUAL
        || compound_token == MINUSEQ
        || compound_token == PLUSEQ
    );
}

// is this token a binary operator?
int is_bin_operator(enum Lexicon compound_token) {
    return (compound_token == ISEQL
        || compound_token == ISNEQL 
        || compound_token == GTEQ 
        || compound_token == LTEQ
        || compound_token == AND
        || compound_token == OR
        || compound_token == GT
        || compound_token == LT
        || compound_token == ADD
        || compound_token == SUB
        || compound_token == MUL
        || compound_token == POW
        || compound_token == NOT
        || compound_token == MOD
        || compound_token == DIV
    );
}


int is_utf(char ch) {
    return ((unsigned char)ch >= 0x80);
}