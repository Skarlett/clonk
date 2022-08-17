#include <string.h>
#include <stdio.h>
#include "lexer.h"

int8_t onk_snprint_token_type(
    char * buf,
    uint16_t buf_max,
    enum onk_lexicon_t token)
{
    const char *fmt = "[%s] ";
    const char *ptok = onk_ptoken(token);

    snprintf(buf, buf_max, fmt, ptok);
    return strlen(ptok) + 3;
}


int onk_snprint_token(
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

int16_t onk_strlen_lex_arr(
    enum onk_lexicon_t *arr,
    uint16_t narr
)
{
    int16_t sum = 0;

    for (uint16_t i=0; narr > i; i++)
        sum += (3 + strlen(onk_ptoken(arr[i])));

    return sum;
}

int16_t onk_strlen_tok_arr(
    struct onk_token_t *arr,
    uint16_t narr
)
{
    uint16_t sum = 0;

    for (uint16_t i=0; narr > i; i++)
        sum += (3 + strlen(onk_ptoken(arr[i].type)));

    return sum;
}

int16_t _onk_snprint_lex_arr(
    char * buf,
    uint16_t nbuf,
    enum onk_lexicon_t token
){
    const char * ptoken = 0;
    char working_buf[ONK_TOK_CHAR_SIZE];
    unsigned char ptoken_len;

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

int16_t onk_snprint_lex_arr(
    char * buf, 
    uint16_t nbuf,
    enum onk_lexicon_t *arr,
    int16_t narr
){
    uint16_t remaining_bytes = nbuf;
    int16_t ptok_len = 0;

    assert(narr > 0);

    for(int16_t i=0; narr > i; i++)
    {
        ptok_len = _onk_snprint_lex_arr(
            buf, remaining_bytes,
            arr[i]);

        if(-1 >= ptok_len)
            return -1;

        remaining_bytes -= ptok_len;
    }

    return nbuf - remaining_bytes;
}

int16_t onk_snprint_rlex_arr(char * buf, uint16_t nbuf, enum onk_lexicon_t **arr, int16_t narr)
{
    uint16_t remaining_bytes = nbuf;
    int16_t ptok_len = 0;

    assert(narr > 0);

    for(int16_t i=0; narr > i; i++)
    {
        ptok_len = _onk_snprint_lex_arr(
            buf, remaining_bytes,
            *arr[i]);

        if(-1 >= ptok_len)
            return -1;

        remaining_bytes -= ptok_len;
    }
    return nbuf - remaining_bytes;
}

int16_t onk_snprint_tokarr_as_lexarr(char * buf, uint16_t nbuf, struct onk_token_t *arr, int16_t narr)
{
    uint16_t remaining_bytes = nbuf;
    int16_t ptok_len = 0;
    assert(narr > 0);

    for(int16_t i=0; narr > i; i++)
    {

        ptok_len = _onk_snprint_lex_arr(
            buf, remaining_bytes,
            arr[i].type);

        if(-1 >= ptok_len)
            return -1;

        remaining_bytes -= ptok_len;
    }

    return nbuf - remaining_bytes;
}

int16_t onk_snprint_rtokarr_as_lexarr(char * buf, uint16_t nbuf, struct onk_token_t **arr, int16_t narr)
{
    uint16_t remaining_bytes = nbuf;
    int16_t ptok_len = 0;

    assert(narr > 0);

    for(int16_t i=0; narr > i; i++)
    {

        ptok_len = _onk_snprint_lex_arr(
            buf, remaining_bytes,
            arr[i]->type);

        if(-1 >= ptok_len)
            return -1;

        remaining_bytes -= ptok_len;
    }

    return nbuf - remaining_bytes;
}
