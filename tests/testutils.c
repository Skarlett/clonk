#include "CuTest.h"
#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/debug.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define FMT_STR "%s\nexpected: \n%s\ngot: \n%s\nsrc: \"%s\""

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

bool seq_eql_ty(
    const struct Token tokens[],
    const enum Lexicon lexicon[]
){
    for (usize i=0 ;; i++) {
        if(lexicon[i] == 0)
            break;
        
        else if (tokens[i].type != lexicon[i])
            return false;
    }
    return true;
}

bool seq_eql_ty_by_ref(
    const struct Token *tokens[],
    const enum Lexicon lexicon[]
){
    for (usize i=0 ;; i++) {
        if (lexicon[i] == 0)
            break;
        else if (tokens[i]->type != lexicon[i])
            return false;
    }
    return true;
}

void AssertTokens(
    CuTest *tc,
    const char *source_code,
    const char *file,
    const char *msg,
    const struct Token tokens[],
    const enum Lexicon answer[]
){
    char uneql_msg[2048];
    char uneql_len_msg[2048];
    
    char got[512];
    char expected[512];
    
    usize len=0;
    for (;; len++) {
        if (answer[len] == 0){
            break;
        }
    }

    sprintf_token_slice(tokens, len, got, 512);
    sprintf_lexicon_slice(answer, len, expected, 512);
    sprintf(uneql_msg, FMT_STR, msg, expected, got, source_code);
    sprintf(uneql_len_msg, "expected len: %ld <\n%s", len, uneql_msg);

}

void AssertTokensByRef(
    CuTest *tc,
    const char *source_code,
    const char *file,
    const char *msg,
    const struct Token *tokens[],
    const enum Lexicon answer[]
){
    char uneql_msg[2048];
    char uneql_len_msg[2048];
    
    char got[512];
    char expected[512];
    
    memset(got, 0, sizeof(char[512]));
    memset(expected, 0, sizeof(char[512]));
    memset(uneql_len_msg, 0, sizeof(char[2048]));
    memset(uneql_msg, 0, sizeof(char[2048]));

    usize len=0;
    for (;; len++) {
        if (answer[len] == 0)
            break;
    }
    if (len == 0)
        return;

    sprintf_token_slice_by_ref(tokens, len, got, 512);
    sprintf_lexicon_slice(answer, len, expected, 512);
    sprintf(uneql_msg, FMT_STR, msg, expected, got, source_code);
    sprintf(uneql_len_msg, "%s: expected len: %ld <\n%s", msg, len, uneql_msg);

}
