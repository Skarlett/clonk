#include "CuTest.h"
#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/debug.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define FMT_STR "%s\nexpected: \n%s\ngot: \n%s\nsrc: \"%s\""

int8_t inner_balance(enum onk_lexicon_t tokens[], uint16_t *tokens_ctr, enum onk_lexicon_t current) {
    //TODO: check for int overflow & buffer
    enum onk_lexicon_t inverted;

    if (onk_is_tok_close_brace(current))
    {
        inverted = invert_brace_tok_ty(current);
        if (inverted == ONK_UNDEFINED_TOKEN)
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

    else if (onk_is_tok_open_brace(current)) {
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
bool is_balanced(struct onk_token_t tokens[], uint16_t ntokens) {
    enum onk_lexicon_t braces[BRACE_BUFFER_SZ];
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


/**
 * Checks if token stream is balanced.
 * To be balanced is every brace opening,
 * having a pairing brace closing token
 * following it eventually.
 * The follow are examples:
 *
 *    a + b + (2 + 5)  Is balanced
 *   (a + b            Is unbalanced.
 *
 * @param tokens array of tokens
 * @param ntokens amount of tokens to read
 *
 * @return bool
 */
//bool is_balanced(struct onk_token_t tokens[], uint16_t ntokens);


/**
 * Checks if token stream is balanced by reference. see (src/parser/lexer/helpers.h#is_balance)
 *
 * @param tokens array of referenced tokens
 * @param ntokens amount of tokens to read
 *
 * @return bool
 */
//bool is_balanced_by_ref(struct onk_token_t *tokens[], uint16_t ntokens);



/*
    function determines if an expression is unbalanced.
    an expression can be unbalanced
    if there is a nonmatching `[` / `(` / `{` character
*/
bool is_balanced_by_ref(struct onk_token_t *tokens[], uint16_t ntokens) {
    enum onk_lexicon_t braces[BRACE_BUFFER_SZ];
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
    const struct onk_token_t tokens[],
    const enum onk_lexicon_t lexicon[]
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
    const struct onk_token_t *tokens[],
    const enum onk_lexicon_t lexicon[]
){
    for (usize i=0 ;; i++) {
        if (lexicon[i] == 0)
            break;
        else if (tokens[i]->type != lexicon[i])
            return false;
    }
    return true;
}

void onk_assert_tokens(
    CuTest *tc,
    const char *source_code,
    const char *file,
    const char *msg,
    const struct onk_token_t tokens[],
    const enum onk_lexicon_t answer[]
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

void onk_assert_tokens_by_ref(
    CuTest *tc,
    const char *source_code,
    const char *file,
    const char *msg,
    const struct onk_token_t *tokens[],
    const enum onk_lexicon_t answer[]
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