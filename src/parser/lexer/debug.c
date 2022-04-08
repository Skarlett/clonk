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
        case ONK_EQUAL_TOKEN: return "eq";
        case ONK_ADD_TOKEN: return "add";
        case ONK_MUL_TOKEN: return "multiply";
        case ONK_DIV_TOKEN: return "divide";
        case ONK_GT_TOKEN: return "greater than";
        case ONK_LT_TOKEN: return "less than";
        case ONK_NOT_EQL_TOKEN: return "is not eq";
        case ONK_ISEQL_TOKEN: return "is eq";
        case ONK_GT_EQL_TOKEN: return "greater than or eq";
        case ONK_LT_EQL_TOKEN: return "less than or eq";
        case ONK_POW_TOKEN: return "exponent";
        case ONK_PLUSEQ_TOKEN: return "plus eq";
        case ONK_MINUS_EQL_TOKEN: return "minus eq";
        case ONK_MOD_TOKEN: return "modolus";
        case ONK_SUB_TOKEN: return "sub";
        case ONK_COLON_TOKEN: return "colon";
        case ONK_SEMICOLON_TOKEN: return "semi-colon";
        case ONK_STRING_LITERAL_TOKEN: return "str_literal";
        case ONK_AMPER_TOKEN: return "&";
        case ONK_PIPE_TOKEN: return "pipe";
        case ONK_AND_TOKEN: return "and";
        case ONK_OR_TOKEN: return "or";
        case ONK_SHR_TOKEN: return "shr";
        case ONK_SHL_TOKEN: return "shl";
        case ONK_BIT_OR_EQL: return "bit or eql";
        case ONK_BIT_AND_EQL: return "bit and eql";
        case ONK_BIT_NOT_EQL: return "bit not eql";
        case ONK_TILDE_TOKEN: return "bit not";
        case ONK_FROM_TOKEN: return "from";
        case ONK_UNDERSCORE_TOKEN: return "underscore";
        case ONK_NOT_TOKEN: return "not";
        case POUND: return "pound";
        case ONK_IF_TOKEN: return "'if";
        case ONK_ELSE_TOKEN: return "'else'";
        case ONK_IMPL_TOKEN: return "'impl'";
        case ONK_DEF_TOKEN: return "'def'";
        case ONK_RETURN_TOKEN: return "'return'";
        //case ONK_AS_TOKEN: return "'as'";
        case ONK_IMPORT_TOKEN: return "'import'";
        case ONK_COMMENT_TOKEN: return "comment";
        case ONK_UNDEFINED_TOKEN: return "undef";
        case ONK_DOT_TOKEN: return "dot";
        default: return "ONK_PTOKEN_UNKNOWN_TOKEN";
    };
}

int8_t onk_snprint_token_type(
    char * buf,
    uint16_t max,
    enum onk_lexicon_t token)
{
    const char *fmt = "[%s] ";
    const char *ptok = onk_ptoken(token);

    snprintf(buf, max, fmt, ptok);
    return strlen(ptok) + 3;
}


int16_t onk_snprint_token(
    char * buf,
    uint16_t max,
    const struct onk_token_t *token)
{
    const char *fmt = "{ start: %ud; end: %ud; seq: %ud; type: %s (%ud) }";
    const char *ptok = 0;
    int nbytes = 0;

    ptok = onk_ptoken(token->type);

    nbytes = snprintf(
         buf,
         max,
         fmt,
         token->start,
         token->end,
         token->seq,
         ptok,
         token->type
    );

    return nbytes;
}

int16_t onk_str_len_lexicon_arr(
    enum onk_lexicon_t *arr,
    uint16_t narr
)
{
    uint16_t sum = 0;

    for (uint16_t i=0; narr > i; i++)
        sum += (3 + strlen(onk_ptoken(arr[i])));

    return sum;
}

int16_t onk_str_len_token_arr(
    struct onk_token_t *arr,
    uint16_t narr
)
{
    uint16_t sum = 0;

    for (uint16_t i=0; narr > i; i++)
        sum += (3 + strlen(onk_ptoken(arr[i].type)));

    return sum;
}

int16_t _onk_snprint_lexicon_arr(char * buf, uint16_t nbuf, enum onk_lexicon_t token) {
    const char * ptoken = 0;
    char working_buf[ONK_TOK_CHAR_SIZE];
    uint8_t ptoken_len;

    ptoken = onk_ptoken(token);
    ptoken_len = strlen(ptoken) + 3;

    if (ptoken_len >= nbuf)
        return -1;

    snprintf(working_buf, ONK_TOK_CHAR_SIZE, "[%s] ", ptoken);

    strncat(
        buf,
        working_buf,
        nbuf
    );

    return ptoken_len;
}

int16_t onk_snprint_lexicon_arr(
    char * buf, uint16_t nbuf,
    enum onk_lexicon_t *arr,
    int16_t narr)
{
    uint16_t remaining_bytes = nbuf;
    int16_t ptok_len = 0;

    assert(narr > 0);

    for(int16_t i=0; narr > i; i++)
    {
        ptok_len = _onk_snprint_lexicon_arr(
            buf, remaining_bytes,
            arr[i]);

        if(-1 >= ptok_len)
            return -1;

        remaining_bytes -= ptok_len;
    }

    return nbuf - remaining_bytes;
}

int16_t onk_snprint_ref_lexicon_arr(char * buf, uint16_t nbuf, enum onk_lexicon_t **arr, int16_t narr)
{
    uint16_t remaining_bytes = nbuf;
    int16_t ptok_len = 0;

    assert(narr > 0);

    for(int16_t i=0; narr > i; i++)
    {
        ptok_len = _onk_snprint_lexicon_arr(
            buf, remaining_bytes,
            *arr[i]);

        if(-1 >= ptok_len)
            return -1;

        remaining_bytes -= ptok_len;
    }
    return nbuf - remaining_bytes;
}

int16_t onk_snprint_tokens_as_lexicon_arr(char * buf, uint16_t nbuf, struct onk_token_t *arr, int16_t narr)
{
    uint16_t remaining_bytes = nbuf;
    int16_t ptok_len = 0;
    assert(narr > 0);

    for(int16_t i=0; narr > i; i++)
    {

        ptok_len = _onk_snprint_lexicon_arr(
            buf, remaining_bytes,
            arr[i].type);

        if(-1 >= ptok_len)
            return -1;

        remaining_bytes -= ptok_len;
    }

    return nbuf - remaining_bytes;
}

int16_t onk_snprint_ref_tokens_as_lexicon_arr(char * buf, uint16_t nbuf, struct onk_token_t **arr, int16_t narr)
{
    uint16_t remaining_bytes = nbuf;
    int16_t ptok_len = 0;

    assert(narr > 0);

    for(int16_t i=0; narr > i; i++)
    {

        ptok_len = _onk_snprint_lexicon_arr(
            buf, remaining_bytes,
            arr[i]->type);

        if(-1 >= ptok_len)
            return -1;

        remaining_bytes -= ptok_len;
    }

    return nbuf - remaining_bytes;
}
