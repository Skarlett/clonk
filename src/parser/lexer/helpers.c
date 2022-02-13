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

int8_t inner_balance(enum Lexicon tokens[], uint16_t *tokens_ctr, enum Lexicon current) {
    //TODO: check for int overflow & buffer
    enum Lexicon inverted;

    if (is_close_brace(current))
    {
        inverted = invert_brace_tok_ty(current);
        if (inverted == TOKEN_UNDEFINED)
          return -1;
        
       /*
            return 1 to stop iteration, 
            and return `is_balanced` as false (0)
        */    
        else if (*tokens_ctr <= 0)
          return 1;

        else if (tokens[*tokens_ctr - 1] == inverted)
          *tokens_ctr -= 1;
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
 *  function determines if an expression is unbalanced.
 *  an expression can be unbalanced 
 *  if there is a nonmatching `[` / `(` / `{` character
*/
bool is_balanced(struct Token tokens[], uint16_t ntokens) {
    enum Lexicon braces[BRACE_BUFFER_SZ];
    uint16_t braces_ctr = 0;
    int8_t ret;

    for (uint16_t i=0; ntokens > i; i++){
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
bool is_balanced_by_ref(struct Token *tokens[], uint16_t ntokens) {
    enum Lexicon braces[BRACE_BUFFER_SZ];
    uint16_t braces_ctr = 0;

    int8_t ret;
    
    for (uint16_t i=0; ntokens > i; i++){
        ret = inner_balance(braces, &braces_ctr, tokens[i]->type);
        
        if (ret == -1)
            return -1;
        
        else if (ret == 1)
            return 0;
    }
    
    return braces_ctr == 0;
}

bool is_num_negative(const char * source, struct Token *token) {
    return token->type == INTEGER && *(source + token->start) == '-';
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
