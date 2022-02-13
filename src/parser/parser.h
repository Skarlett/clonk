#ifndef _HEADER_EXPR__
#define _HEADER_EXPR__

#include <stdint.h>
#include "../utils/vec.h"
#include "lexer/lexer.h"

struct ParserInput {
    const char * src_code;
    uint16_t src_code_sz;
    struct Vec tokens;

    bool add_glob_scope;
};

struct ParserOutput {
    /* Vec<struct Token *> */
    const struct Vec postfix;

    /* Vec<struct Token> */
    struct Vec token_pool;

    /* Vec<struct ParseError> */
    struct Vec errors;
};

enum ParserError_t {
    parse_err_unexpected_token,
};


struct UnexpectedTokError {
    /* Lexicon[N] null-terminated malloc */
    enum Lexicon *expected;
    uint16_t nexpected;

    struct Token thrown;
};

struct ParserError {
    enum ParserError_t type;

    /* window of tokens effected */
    struct TokenSelection window;

    union {
        struct UnexpectedTokError unexpected_tok;

    } error;
};

/*
  Shunting yard expression parsing algorthim 
  https://en.wikipedia.org/wiki/Shunting-yard_algorithm
  --------------

  This function takes takes a stream of token `tokens[]`
  and writes an array of pointers (of type `struct Token`)
  into `*output[]` in postfix notation.

  The contents of `*output[]` will be a
  POSTFIX notation referenced from the 
  INFIX notation of `input[]`.

    infix: 1 + 1
  postfix: 1 1 +
    input: [INT, ADD, INT]
   output: [*INT, *INT, *ADD]

  Further more, this function handles organizing operation precedense
  based on shunting-yard algorthm.
  This is in combination with arithmetic operations, and our custom operations
  (GROUP, INDEX_ACCESS, APPLY, DOT).
  Upon completion, the result will be an ordered array of operands, 
  and operators ready to be evaluated into a tree structure.

    infix: (1+2) * (1 + 1)
  postfix: 1 2 + 1 1 + *
  To turn the output into a tree see `stage_postfix_parser`.

  Digging deeper into the realm of this, 
  you'll find I evaluate some custom operators
  such as the DOT token, and provide 
  extra operators to the output to describe
  function calls (APPLY(N)).

  infix:   foo(a, b+c).bar(1)
  postfix  foo a b c + APPLY(3) bar 1 APPLY(2) .
  pretty-postfix:
           ((foo a (b c +) APPLY(3)) bar 1 APPLY(2) .)
*/

int8_t parse(
    struct ParserInput *input,
    struct ParserOutput *out
);

int8_t parser_free(struct Parser *state);
int8_t parser_reset(struct Parser *state);

int8_t is_token_unexpected(struct Parser *state);


#endif
