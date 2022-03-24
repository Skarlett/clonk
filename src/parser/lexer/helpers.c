#include <stdbool.h>
#include "lexer.h"

bool onk_is_tok_delimiter(enum onk_lexicon_t token) {
    return token > __ONK_MARKER_DELIM_START
        && __ONK_MARKER_DELIM_END > token;
}

bool onk_is_tok_brace(enum onk_lexicon_t token) {
    return token > __ONK_MARKER_BRACE_START
        && __ONK_MARKER_BRACE_END > token;
}

bool onk_is_tok_close_brace(enum onk_lexicon_t token) {
    return token > __ONK_MARKER_CLOSE_BRACE_START
        && __ONK_MARKER_CLOSE_BRACE_END > token;
}

bool onk_is_tok_open_brace(enum onk_lexicon_t token) {
    return (token > __ONK_MARKER_OPEN_BRACE_START
      && __ONK_MARKER_OPEN_BRACE_END > token);
}

bool onk_is_tok_unit(enum onk_lexicon_t token) {
    return token > __ONK_MARKER_UNIT_START
        && __ONK_MARKER_UNIT_END > token;
}

bool onk_is_tok_asn_operator(enum onk_lexicon_t token) {
    return token > __ONK_MARKER_ASN_START
        && __ONK_MARKER_ASN_END > token;
}

bool onk_is_tok_binop(enum onk_lexicon_t token) {
    return token > __ONK_MARKER_BIN_START
        && __ONK_MARKER_BIN_END > token;
}

bool onk_is_tok_unary_operator(enum onk_lexicon_t token) {
    return token > __ONK_MARKER_UNARY_START
        && __ONK_MARKER_UNARY_END > token;
}

bool onk_is_tok_operator(enum onk_lexicon_t token) {
    return token > __ONK_MARKER_OP_START
        && __ONK_MARKER_OP_END > token;
}

bool onk_is_tok_block_keyword(enum onk_lexicon_t token) {
    return token > __ONK_MARKER_KEYWORD_BLOCK_START
        && __ONK_MARKER_KEYWORD_BLOCK_END > token;
}

bool onk_is_tok_data_keyword(enum onk_lexicon_t token){
    return token > __ONK_MARKER_KEYWORD_DATA_START
        && __ONK_MARKER_KEYWORD_DATA_END > token;
}

bool onk_is_tok_keyword(enum onk_lexicon_t token){
    return onk_is_tok_block_keyword(token)
        || onk_is_tok_data_keyword(token);
}

bool _onk_do_default_expectation(enum onk_lexicon_t token){
    return token > __ONK_MARKER_KEYWORD_DATA_START
        && __ONK_MARKER_KEYWORD_DATA_END > token;
}

bool onk_is_tok_illegal(enum onk_lexicon_t token){
    return token > __ONK_MARKER_ILLEGAL_INPUT_START
        && __ONK_MARKER_ILLEGAL_INPUT_END > token;
}


/* keywords that can only be used in blocks */
/* bool onk_is_tok_kword_block(enum onk_lexicon_t token) { */
/*     return token > __ONK_MARKER_KEYWORD_BLOCK_START */
/*         && __ONK_MARKER_KEYWORD_BLOCK_END > token; */
/* } */

bool _onk_is_group(enum onk_lexicon_t tok) {
    return tok > __ONK_MARKER_GROUP_START
        && __ONK_MARKER_GROUP_END > tok;
}

bool onk_is_tok_group_modifier(enum onk_lexicon_t tok) {
    return tok > __ONK_MARKER_GROUP_OP_START
        && __ONK_MARKER_GROUP_OP_END > tok;
}

bool onk_is_tok_loopctlkw(enum onk_lexicon_t tok) {
    return tok > __ONK_MARKER_LOOP_CTL_START
        && __ONK_MARKER_LOOP_CTL_END > tok;
}

bool onk_is_tok_whitespace(enum onk_lexicon_t tok) {
    return tok > __ONK_MARKER_WHITESPACE_START
        && __ONK_MARKER_WHITESPACE_END > tok;
}

bool onk_is_int_tok_negative(const char * source, struct onk_token_t *token) {
    return token->type == ONK_INTEGER_TOKEN
        && *(source + token->start) == '-';
}

bool onk_is_utf_byte(char ch) {
    unsigned char copy = ch;

    return (copy >= 0x80
        /* accept a cup of tea (£) */
        && copy != 0xa3);
}

/* null delimitated */
uint8_t onk_eq_any_tok(enum onk_lexicon_t cmp, enum onk_lexicon_t buffer[]) {
  for (uint16_t i=0 ;; i++)
    if(buffer[i] == 0) break;
    else if (buffer[i] == cmp)
      return i;
  
  return 0;
}

/*
    Inverts brace characters into their counter parts.
    example
       input:"(" - outputs:")"
       input:"]" - output:"["
*/
enum onk_lexicon_t onk_invert_brace(enum onk_lexicon_t token) {
    switch (token) {
        case ONK_PARAM_OPEN_TOKEN: return ONK_PARAM_CLOSE_TOKEN;
        case ONK_PARAM_CLOSE_TOKEN: return ONK_PARAM_OPEN_TOKEN;
        case ONK_BRACE_OPEN_TOKEN: return ONK_BRACE_CLOSE_TOKEN;
        case ONK_BRACE_CLOSE_TOKEN: return ONK_BRACE_OPEN_TOKEN;
        case ONK_BRACKET_CLOSE_TOKEN: return ONK_BRACKET_OPEN_TOKEN;
        case ONK_BRACKET_OPEN_TOKEN: return ONK_BRACKET_CLOSE_TOKEN;
        case ONK_HASHMAP_LITERAL_START_TOKEN: return ONK_BRACE_CLOSE_TOKEN;
        default: return ONK_UNDEFINED_TOKEN;
    }
}

