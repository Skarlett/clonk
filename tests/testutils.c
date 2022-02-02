#include "CuTest.h"
#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/debug.h"
#include "../src/prelude.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define FMT_STR "%s\nexpected: \n%s\ngot: \n%s\nsrc: \"%s\""

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
    int line,
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

    CuAssert_Line(tc, file, line, uneql_len_msg, tokens[len-1].type == answer[len-1]);
    CuAssert_Line(tc, file, line, uneql_msg, seq_eql_ty(tokens, answer) == 1);
}

void AssertTokensByRef(
    CuTest *tc,
    const char *source_code,
    const char *file,
    int line,
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

    CuAssert_Line(tc, file, line, uneql_len_msg, tokens[len-1]->type == answer[len-1]);
    CuAssert_Line(tc, file, line, uneql_msg, seq_eql_ty_by_ref(tokens, answer) == 1);
}
