#include <stdbool.h>
#include "lexer.h"

#define BRACE_BUFFER_SZ 256

bool onk_is_tok_delimiter(enum onk_lexicon_t token) {
    return token > __MARKER_DELIM_START
        && __MARKER_DELIM_END > token;
}

bool onk_is_tok_close_brace(enum onk_lexicon_t token)
{
    return token > __MARKER_CLOSE_BRACE_START
        && __MARKER_CLOSE_BRACE_END > token;
}

bool onk_is_tok_open_brace(enum onk_lexicon_t token) {
    return token > __MARKER_OPEN_BRACE_START
        && __MARKER_OPEN_BRACE_END > token;
}

bool onk_is_tok_unit(enum onk_lexicon_t token) {
    return token > __MARKER_UNIT_START
        && __MARKER_UNIT_END > token;
}

bool is_asn_operator(enum onk_lexicon_t token) {
    return token > __MARKER_ASN_START
        && __MARKER_ASN_END > token;
}

/*
   returns bool if token is a binary operator
*/
bool onk_is_tok_operator(enum onk_lexicon_t token) {
    return token > __MARKER_OP_START
        && __MARKER_OP_END > token;
}

/* null delimitated */
uint8_t onk_eq_any_tok(enum onk_lexicon_t cmp, enum onk_lexicon_t buffer[]) {
  for (uint16_t i=0 ;; i++)
    if(buffer[i] == 0) break;
    else if (buffer[i] == cmp)
      return i;
  
  return 0;
}

/* is character utf encoded */
bool onk_is_utf_byte(char ch) {
    return ((unsigned char)ch >= 0x80);
}

/*
    Inverts brace characters into their counter parts.
    example
       input:"(" - outputs:")"
       input:"]" - output:"["
*/
enum onk_lexicon_t invert_brace_tok_ty(enum onk_lexicon_t token) {
    switch (token) {
        case ONK_PARAM_OPEN_TOKEN: return ONK_PARAM_CLOSE_TOKEN;
        case ONK_PARAM_CLOSE_TOKEN: return ONK_PARAM_OPEN_TOKEN;
        case ONK_BRACE_OPEN_TOKEN: return ONK_BRACE_CLOSE_TOKEN;
        case ONK_BRACE_CLOSE_TOKEN: return ONK_BRACE_OPEN_TOKEN;
        case ONK_BRACKET_CLOSE_TOKEN: return ONK_BRACKET_OPEN_TOKEN;
        case ONK_BRACKET_OPEN_TOKEN: return ONK_BRACKET_CLOSE_TOKEN;
        default: return ONK_TOKEN_UNDEFINED;
    }
}

bool onk_is_int_tok_negative(const char * source, struct onk_token_t *token) {
    return token->type == ONK_INTEGER_TOKEN
        && *(source + token->start) == '-';
}

bool onk_is_tok_keyword(enum onk_lexicon_t token) {
    return token > __MARKER_KEYONK_WORD_TOKEN_START
        && __MARKER_KEYONK_WORD_TOKEN_END > token;
}

bool is_group(enum onk_lexicon_t tok)
{
    return tok > __MARKER_GROUP_START
        && __MARKER_GROUP_END > tok;
}

bool onk_is_tok_group_modifier(enum onk_lexicon_t tok) {
    return tok > __MARKER_GROUP_OP_START
        && __MARKER_GROUP_OP_END > tok;
}
