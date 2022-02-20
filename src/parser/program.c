#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "lexer/lexer.h"
#include "../prelude.h"

#include "expr/expr.h"

struct Program {
    const char * src_code;


    /* Vec<onk_token_t> */
    struct onk_vec_t tokens;
    struct Expr *ast;

    /* Vec<Error> */
    struct onk_vec_t errors;

};


struct ParseOutput {
    enum ParserOutput status;

    /* tokens[] on heap */
    struct onk_token_t * tokens;

    union {
        struct OKOutput ok;
        struct ErrorOutput err;
    } pkg;
};


int parse_code(
    struct ParseInput *input,
    /* Vec<struct Expr> */
    struct ParseOutput *output
){

    struct Parser prefix_stage;
    struct onk_lexer_input_t lex_input;


    assert(onk_tokenize(
        input->source_code,
        input->tokens,
        &token_ctr,
        24123,
        true,
        NULL,
    ) == 0);

    parse_expr(
        src_code, 
        tokens,
        tokens_ctr,
        &prefix_stage
        
        )
}
