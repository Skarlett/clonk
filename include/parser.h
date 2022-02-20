#ifndef __ONK_PARSER_HEADER__
#define __ONK_PARSER_HEADER__

#include "lexer.h"

struct ParserInput {
    const char * src_code;
    uint16_t src_code_sz;
    struct Vec tokens;

    bool add_glob_scope;
};

struct ParserOutput {
    /* Vec<struct onk_token_t *> */
    const struct Vec postfix;

    /* Vec<struct onk_token_t> */
    struct Vec token_pool;

    /* Vec<struct ParseError> */
    struct Vec errors;
};

enum ParserError_t {
    parse_err_unexpected_token
};


struct UnexpectedTokError {
    /* onk_lexicon_t[N] null-terminated malloc */
    enum onk_lexicon_t *expected;
    uint16_t nexpected;

    struct onk_token_t thrown;
};

struct ParserError {
    enum ParserError_t type;

    /* window of tokens effected */
    struct onk_token_selection_t window;

    union {
        struct UnexpectedTokError unexpected_tok;

    } error;
};

/*
  Shunting yard expression parsing algorthim 
  https://en.wikipedia.org/wiki/Shunting-yard_algorithm
  --------------

  This function takes takes a stream of token `tokens[]`
  and writes an array of pointers (of type `struct onk_token_t`)
  into `*output[]` in postfix notation.

  The contents of `*output[]` will be a
  POSTFIX notation referenced from the 
  INFIX notation of `input[]`.

    infix: 1 + 1
  postfix: 1 1 +
    input: [INT, ONK_ADD_TOKEN, INT]
   output: [*INT, *INT, *ONK_ADD_TOKEN]

  Further more, this function handles organizing operation precedense
  based on shunting-yard algorthm.
  This is in combination with arithmetic operations, and our custom operations
  (GROUP, INDEX_ACCESS, APPLY, ONK_DOT_TOKEN).
  Upon completion, the result will be an ordered array of operands, 
  and operators ready to be evaluated into a tree structure.

    infix: (1+2) * (1 + 1)
  postfix: 1 2 + 1 1 + *
  To turn the output into a tree see `stage_postfix_parser`.

  Digging deeper into the realm of this, 
  you'll find I evaluate some custom operators
  such as the ONK_DOT_TOKEN token, and provide 
  extra operators to the output to describe
  function calls (APPLY(N)).

  infix:   foo(a, b+c).bar(1)
  postfix  foo a b c + APPLY(3) bar 1 APPLY(2) .
  pretty-postfix:
           ((foo a (b c +) APPLY(3)) bar 1 APPLY(2) .)
*/

int8_t onk_parse(
    struct ParserInput *input,
    struct ParserOutput *out
);

#endif
