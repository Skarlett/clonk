#include <stdbool.h>
#include "lexer.h"

#define BRACE_BUFFER_SZ 256

bool is_delimiter(enum Lexicon token) {
    return token > __MARKER_DELIM_START
        && __MARKER_DELIM_END > token;
}

bool is_close_brace(enum Lexicon token)
{
    return token > __MARKER_CLOSE_BRACE_START
        && __MARKER_CLOSE_BRACE_END > token;
}

bool is_open_brace(enum Lexicon token) {
    return token > __MARKER_OPEN_BRACE_START
        && __MARKER_OPEN_BRACE_END > token;
}

bool is_unit(enum Lexicon token) {
    return token > __MARKER_UNIT_START
        && __MARKER_UNIT_END > token;
}

bool is_asn_operator(enum Lexicon token) {
    return token > __MARKER_ASN_START
        && __MARKER_ASN_END > token;
}

/*
   returns bool if token is a binary operator
*/
bool is_operator(enum Lexicon token) {
    return token > __MARKER_OP_START
        && __MARKER_OP_END > token;
}

/* null delimitated */
uint8_t eq_any_tok(enum Lexicon cmp, enum Lexicon buffer[]) {
  for (uint16_t i=0 ;; i++)
    if(buffer[i] == 0) break;
    else if (buffer[i] == cmp)
      return i;
  
  return 0;
}

/* is character utf encoded */
bool is_utf(char ch) {
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
        default: return TOKEN_UNDEFINED;
    }
}

bool is_num_negative(const char * source, struct Token *token) {
    return token->type == INTEGER
        && *(source + token->start) == '-';
}

bool is_keyword(enum Lexicon token) {
    return token > __MARKER_KEYWORD_START
        && __MARKER_KEYWORD_END > token;
}

bool is_group(enum Lexicon tok)
{
    return tok > __MARKER_GROUP_START
        && __MARKER_GROUP_END > tok;
}

bool is_group_modifier(enum Lexicon tok) {
    return tok > __MARKER_GROUP_OP_START
        && __MARKER_GROUP_OP_END > tok;
}
