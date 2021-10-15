#include <string.h>
#include <stdio.h>
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
        case STRING_LITERAL: return "str_literal";
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
        case UNDEFINED: return "undef";
        case DOT: return "dot";
        default: return "PTOKEN_ERROR_UNKNOWN_TOKEN";
    };
}

char brace_as_char(enum Lexicon tok) {
    switch(tok){
        case BRACE_OPEN: return '{';
        case BRACE_CLOSE: return '}';
        case PARAM_OPEN: return '(';
        case PARAM_CLOSE: return ')';
        case BRACKET_OPEN: return '[';
        case BRACKET_CLOSE: return ']';
        default:
            return -1;
    }
}

char invert_brace_char(char brace) {
    switch(brace){
        case '{': return '}';
        case '}': return '{';
        case '(': return ')';
        case ')': return '(';
        case '[': return ']';
        case ']': return '[';
        default:
            return -1;
    }
}


int8_t __sprintf_token_ty_slice(char *output, usize output_sz, enum Lexicon token, usize *ctr) {
    char token_buf[64];
    const char *ptok;

    if (!ctr || !output)
        return -1;
    
    ptok = ptoken(token);
    sprintf(token_buf, "[%s], ", ptok);
        
    memcpy(
        output + *ctr,
        token_buf,
        strlen(ptok) + 4
    );

    *ctr += strlen(ptok) + 4;
    if (*ctr > output_sz)
        return -1;
    
    return 0;
}

int8_t sprintf_token_slice(
    const struct Token tokens[],
    usize ntokens,
    char * output,
    usize output_sz    
){
    usize ctr=0;
    
    if (!tokens || !output)
        return -1;
    
    output[0] = '[';

    for (usize i=0; ntokens > i; i++) {
        if (__sprintf_token_ty_slice(
                output+1,
                output_sz,
                tokens[i].type,
                &ctr
            ) == -1
        ) return -1;
    }

    if (output_sz > strlen(output)+2) {
        output[strlen(output)] = ']';
        output[strlen(output)+1] = 0;
        return -1;
    }
    
    return 0;
}

int8_t sprintf_lexicon_slice(
    const enum Lexicon tokens[],
    usize ntokens,
    char * output,
    usize output_sz    
){
    if (!tokens || !output)
        return -1;
    
    usize ctr=0;

    output[0] = '[';

    for (usize i=0; ntokens > i; i++) {
        if (__sprintf_token_ty_slice(
                output+1,
                output_sz,
                tokens[i],
                &ctr
            ) == -1
        ) return -1;
    }

    if (output_sz > strlen(output)+2) {
        output[strlen(output)] = ']';
        output[strlen(output)+1] = 0;
        return -1;
    }
    
    return 0;
}

int8_t sprintf_token_slice_by_ref(
    const struct Token *tokens[],
    usize ntokens,
    char * output,
    usize output_sz    
){
    if (!output || !tokens)
        return -1;

    usize ctr=0;
    output[0] = '[';

    for (usize i=0; ntokens > i; i++) {        
        if (__sprintf_token_ty_slice(
                output+1,
                output_sz-1,
                tokens[i]->type,
                &ctr
            ) == -1
        ) return -1;
    }

    if (output_sz > strlen(output)+2) {
        return -1;
    }
    output[strlen(output)] = ']';
    output[strlen(output)+1] = 0;
    
    return 0;
}

int8_t sprint_src_code(
    char * output,
    usize output_sz,
    usize *nbytes,
    const char * source,
    const struct Token *token

) {
    if (!source 
        || !output 
        || !token 
        || token->start > token->end
        || token->end - token->start > output_sz
    ) return -1;
    
    memcpy(
        output,
        source+token->start,
        token->end-token->start
    );
    
    *nbytes = token->end - token->start;
    return 0;
}