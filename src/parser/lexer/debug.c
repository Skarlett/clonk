#include <string.h>
#include <stdio.h>
#include "lexer.h"

const char * onk_ptoken(enum onk_lexicon_t t) {
    switch (t) {
        case ONK_INTEGER_TOKEN: return "integer";
        case ONK_WORD_TOKEN: return "word";
        case ONK_CHAR_TOKEN: return "char";
        case ONK_NULL_TOKEN: return "nulltoken";
        case ONK_WHITESPACE_TOKEN: return "whitespace";
        case ONK_BRACE_OPEN_TOKEN: return "brace_open";
        case ONK_BRACE_CLOSE_TOKEN: return "brace_close";
        case ONK_PARAM_OPEN_TOKEN: return "param_open";
        case ONK_PARAM_CLOSE_TOKEN: return "param_close";
        case ONK_COMMA_TOKEN: return "comma";
        case ONK_DIGIT_TOKEN: return "digit";
        case ONK_DOUBLE_QUOTE_TOKEN: return "d_quote";
        case EQUAL: return "eq";
        case ONK_ADD_TOKEN: return "add";
        case ONK_MUL_TOKEN: return "multiply";
        case ONK_DIV_TOKEN: return "divide";
        case GT: return "greater than";
        case LT: return "less than";
        case ISNEQL: return "is not eq";
        case ISEQL: return "is eq";
        case GTEQ: return "greater than or eq";
        case LTEQ: return "less than or eq";
        case ONK_POW_TOKEN: return "exponent";
        case PLUSEQ: return "plus eq";
        case MINUSEQ: return "minus eq";
        case ONK_MOD_TOKEN: return "modolus";
        case ONK_SUB_TOKEN: return "sub";
        case ONK_COLON_TOKEN: return "colon";
        case ONK_SEMICOLON_TOKEN: return "semi-colon";
        case ONK_STRING_LITERAL_TOKEN: return "str_literal";
        case AMPER: return "&";
        case PIPE: return "pipe";
        case AND: return "and";
        case OR: return "or";
        case SHR: return "shr";
        case SHL: return "shl";
        case PIPEOP: return "pipe op";
        case BOREQL: return "bit or eql";
        case BANDEQL: return "bit and eql";
        case BNEQL: return "bit not eql";
        case ONK_TILDE_TOKEN: return "bit not";
        case FROM: return "from";
        case ONK_UNDERSCORE_TOKEN: return "underscore";
        case ONK_NOT_TOKEN: return "not";
        case POUND: return "pound";
//        case STATIC: return "'static'";
//        case CONST: return "'const'";
        case IF: return "'if";
        case ELSE: return "'else'";
//        case IMPL: return "'impl'";
        case FUNC_DEF: return "'def'";
        case RETURN: return "'return'";
//        case AS: return "'as'";
        case ONK_ATSYM__TOKEN: return "@";
        case IMPORT: return "'import'";
//        case EXTERN: return "'extern'";
        case ONK_COMMENT_TOKEN: return "comment";
        case ONK_TOKEN_UNDEFINED: return "undef";
        case ONK_DOT_TOKEN: return "dot";
        default: return "ONK_PTOKEN_ERROR_UNKNOWN_TOKEN";
    };
}

/* char brace_as_char(enum onk_lexicon_t tok) { */
/*     switch(tok){ */
/*         case ONK_BRACE_OPEN_TOKEN: return '{'; */
/*         case ONK_BRACE_CLOSE_TOKEN: return '}'; */
/*         case ONK_PARAM_OPEN_TOKEN: return '('; */
/*         case ONK_PARAM_CLOSE_TOKEN: return ')'; */
/*         case ONK_BRACKET_OPEN_TOKEN: return '['; */
/*         case ONK_BRACKET_CLOSE_TOKEN: return ']'; */
/*         default: */
/*             return -1; */
/*     } */
/* } */

/* not used */
/* char invert_brace_char(char brace) { */
/*     switch(brace){ */
/*         case '{': return '}'; */
/*         case '}': return '{'; */
/*         case '(': return ')'; */
/*         case ')': return '('; */
/*         case '[': return ']'; */
/*         case ']': return '['; */
/*         default: */
/*             return -1; */
/*     } */
/* } */


char * __sprintf_token_ty_slice(char *output, uint16_t output_sz, enum onk_lexicon_t token, uint16_t *ctr) {
    char token_buf[64];
    const char *ptok;

    if (!ctr || !output)
        return 0;
    
    ptok = onk_ptoken(token);
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
    const enum onk_lexicon_t *lex_arr;
    const struct onk_token_t *tok_arr;
    const struct onk_token_t **tok_arr_by_ref;
};

char * __sprintf_inner(
    uint16_t ntokens,
    char *output, uint16_t output_sz,
    union SPFData *ptr, enum SPFMode spf_ty
){
    uint16_t ctr = 0;
    enum onk_lexicon_t item = 0;

    if (!ptr || !output)
        return 0;
    
    output[0] = '[';

    for (uint16_t i=0; ntokens > i; i++) {

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
    const struct onk_token_t tokens[],
    uint16_t ntokens,
    char * output,
    uint16_t output_sz    
){
    uint16_t ctr=0;
    union SPFData input;
    enum SPFMode mode = spf_tok_arr;
    input.tok_arr = tokens;

    if (__sprintf_inner(ntokens, output, output_sz, &input, mode) == 0)
        return -1;

    return 0;
}

int8_t sprintf_lexicon_slice(
    const enum onk_lexicon_t tokens[],
    uint16_t ntokens,
    char * output,
    uint16_t output_sz
){
    union SPFData input;
    enum SPFMode mode = spf_lex_arr;
    input.lex_arr = tokens;

    if (__sprintf_inner(ntokens, output, output_sz, &input, mode) == 0)
        return -1;

    return 0;

}

int8_t sprintf_token_slice_by_ref(
    const struct onk_token_t *tokens[],
    uint16_t ntokens,
    char * output,
    uint16_t output_sz    
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
    uint16_t output_sz,
    uint16_t *nbytes,
    const char * source,
    const struct onk_token_t *token

){
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
