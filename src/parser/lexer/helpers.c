#include "lexer.h"
#include "../../prelude.h"

#define BRACE_BUFFER_SZ 256

int8_t is_close_brace(enum Lexicon token) {
    return (token == PARAM_CLOSE 
    || token == BRACE_CLOSE  
    || token == BRACKET_CLOSE);
}

int8_t is_open_brace(enum Lexicon token) {
    return (token == PARAM_OPEN 
    || token == BRACE_OPEN  
    || token == BRACKET_OPEN);
}

int8_t is_data(enum Lexicon token) {
    return (token == WORD
        || token == INTEGER 
        || token == STRING_LITERAL
    );
}

int8_t is_fncall(struct Token tokens[], usize ntokens) {
    return ntokens > 1 
        && tokens[0].type == WORD 
        || tokens[1].type == PARAM_OPEN;
}

int8_t is_cmp_operator(enum Lexicon token) {
    return (
        token == ISEQL
        || token == ISNEQL  
        || token == GTEQ 
        || token == LTEQ
        || token == AND
        || token == OR
    );
}

int8_t is_assignment_operator(enum Lexicon token) {
    return (
        token == EQUAL
        || token == MINUSEQ
        || token == PLUSEQ
    );
}
/*
    returns bool if token is a binary operator
*/
int8_t is_bin_operator(enum Lexicon token) {
    return (token == ISEQL
        || token == ISNEQL 
        || token == GTEQ 
        || token == LTEQ
        || token == AND
        || token == OR
        || token == GT
        || token == LT
        || token == ADD
        || token == SUB
        || token == MUL
        || token == POW
        || token == NOT
        || token == MOD
        || token == DIV
        || token == DOT
    );
}

/* is character utf encoded */
int8_t is_utf(char ch) {
    return ((unsigned char)ch >= 0x80);
}

/*
    Inverts brace characters into their counter parts.
    example
       input:"(" - outputs:")"
       input:"]" - output:"["
*/
enum Lexicon invert_brace_tok_ty(enum Lexicon token) {
    switch (token) {
        case PARAM_OPEN: return PARAM_CLOSE;
        case PARAM_CLOSE: return PARAM_OPEN;
        case BRACE_OPEN: return BRACE_CLOSE;
        case BRACE_CLOSE: return BRACE_OPEN;
        case BRACKET_CLOSE: return BRACKET_OPEN;
        case BRACKET_OPEN: return BRACKET_CLOSE;
        default: return UNDEFINED;
    }
}

int8_t inner_balance(enum Lexicon tokens[], uint16_t *tokens_ctr, enum Lexicon current) {
    //todo: check for int overflow & buffer
    enum Lexicon inverted;

    if (is_close_brace(current)) {
        inverted = invert_brace_tok_ty(current);

        if (inverted == -1)
            return -1;
        
        /* 
            return 1 to stop iteration,  and return `is_balanced` as false (0)
        */    
        else if (*tokens_ctr <= 0)
            return 1;

        else if (tokens[(*tokens_ctr)-1] == inverted)
            (*tokens_ctr)--;
    }

    else if (is_open_brace(current)) {
        if (*tokens_ctr >= BRACE_BUFFER_SZ) {
            return -1;
        }
        
        *tokens_ctr += 1;
        tokens[(*tokens_ctr)-1] = current;
    }

    return 0;
}

/* 
    function determines if an expression is unbalanced.
    an expression can be unbalanced 
    if there is a nonmatching `[` / `(` / `{` character
*/
int8_t is_balanced(struct Token tokens[], usize ntokens) {
    enum Lexicon braces[BRACE_BUFFER_SZ];
    uint16_t braces_ctr = 0;
    int8_t ret;

    for (usize i=0; ntokens > i; i++){
        ret = inner_balance(braces, &braces_ctr, tokens[i].type);
        if (ret == -1)
            return -1;
        
        // immediately unbalanced
        else if (ret == 1)
            return 0;
    }

    return braces_ctr == 0;
}


/* 
    function determines if an expression is unbalanced.
    an expression can be unbalanced 
    if there is a nonmatching `[` / `(` / `{` character
*/
int8_t is_balanced_by_ref(struct Token *tokens[], usize ntokens) {
    enum Lexicon braces[BRACE_BUFFER_SZ];
    uint16_t braces_ctr = 0;

    int8_t ret;
    
    for (usize i=0; ntokens > i; i++){
        ret = inner_balance(braces, &braces_ctr, tokens[i]->type);
        
        if (ret == -1)
            return -1;
        
        else if (ret == 1)
            return 0;
    }
    
    return braces_ctr == 0;
}

int8_t is_keyword(enum Lexicon token) {
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