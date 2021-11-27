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
        case RETURN: return "'return'";
        case AS: return "'as'";
        case ATSYM: return "@";
        case IMPORT: return "'import'";
        case EXTERN: return "'extern'";
        case COMMENT: return "comment";
        case TOKEN_UNDEFINED: return "undef";
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


char * __sprintf_token_ty_slice(char *output, usize output_sz, enum Lexicon token, usize *ctr) {
    char token_buf[64];
    const char *ptok;

    if (!ctr || !output)
        return 0;
    
    ptok = ptoken(token);
    sprintf(token_buf, "[%s], ", ptok);
    
    return strncat(output, token_buf, strlen(ptok)+4);
}

enum SPFMode {
    BTypeNull,
    spf_lex_arr,
    spf_tok_arr,
    spf_tok_arr_by_ref
};

union SPFData {
    const enum Lexicon *lex_arr;
    const struct Token *tok_arr;
    const struct Token **tok_arr_by_ref;
};

char * __sprintf_inner(
    usize ntokens,
    char *output, usize output_sz,
    union SPFData *ptr, enum SPFMode spf_ty) {
    usize ctr = 0;
    enum Lexicon item = 0;

    if (!ptr || !output)
        return 0;
    
    output[0] = '[';

    for (usize i=0; ntokens > i; i++) {

        if (spf_ty == spf_lex_arr && ptr->lex_arr) 
            item = ptr->lex_arr[i];
        else if (spf_ty == spf_tok_arr && ptr->tok_arr)
            item = ptr->tok_arr[i].type;
        else if (spf_ty == spf_tok_arr_by_ref && ptr->tok_arr_by_ref)
            item = ptr->tok_arr_by_ref[i]->type;
        else return 0;
        
        if (__sprintf_token_ty_slice(
          output,
          output_sz,
          item,
          &ctr
        ) == 0) return 0;

    }

    strncat(output, "]", 1);
    
    return 0;
}


int8_t sprintf_token_slice(
    const struct Token tokens[],
    usize ntokens,
    char * output,
    usize output_sz    
){
    usize ctr=0;
    union SPFData input;
    enum SPFMode mode = spf_tok_arr;
    input.tok_arr = tokens;

    if (__sprintf_inner(ntokens, output, output_sz, &input, mode) == 0)
        return -1;

    return 0;
}

int8_t sprintf_lexicon_slice(
    const enum Lexicon tokens[],
    usize ntokens,
    char * output,
    usize output_sz
){
    union SPFData input;
    enum SPFMode mode = spf_lex_arr;
    input.lex_arr = tokens;

    if (__sprintf_inner(ntokens, output, output_sz, &input, mode) == 0)
        return -1;

    return 0;

}

int8_t sprintf_token_slice_by_ref(
    const struct Token *tokens[],
    usize ntokens,
    char * output,
    usize output_sz    
){
    union SPFData input;
    enum SPFMode mode = spf_tok_arr_by_ref;
    input.tok_arr_by_ref = tokens;

    if (__sprintf_inner(ntokens, output, output_sz, &input, mode) == 0)
        return -1;

    return 0;
}

int8_t sprint_src_code(
    char * output,
    usize output_sz,
    usize *nbytes,
    const char * source,
    const struct Token *token

) {
    if (!source || !output || !token 
        || token->start > token->end
        || token->end - token->start > output_sz)
        return -1;
    
    memcpy(
        output,
        source+token->start,
        token->end-token->start
    );
    
    *nbytes = token->end - token->start;
    return 0;
}