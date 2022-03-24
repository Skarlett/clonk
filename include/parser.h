#ifndef __ONK_PARSER_HEADER__
#define __ONK_PARSER_HEADER__

#include "lexer.h"

struct onk_parser_input_t {
    const char * src_code;
    uint16_t src_code_sz;
    struct onk_vec_t tokens;

    bool add_glob_scope;
};

struct parser_output_t {
    /* Vec<struct onk_token_t *> */
    struct onk_vec_t postfix;

    /* Vec<struct onk_token_t> */
    struct onk_vec_t token_pool;

    /* Vec<struct ParseError> */
    struct onk_vec_t errors;

    bool stage_failed;
};

enum onk_parse_err_t {
    parse_err_unexpected_token
};

struct UnexpectedTokError {
    /* onk_lexicon_t[N] null-terminated malloc */
    enum onk_lexicon_t *expected;
    uint16_t nexpected;

    struct onk_token_t thrown;
};

struct ParserError {
    enum onk_parse_err_t type;

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
    struct onk_parser_input_t *input,
    struct parser_output_t *out
);

struct onk_parse_group_t {
    //TODO: Implement this
    uint16_t seq;

    // amount of delimiters
    uint16_t delimiter_cnt;

    /*
    ** amount of expressions
     * to consume off the stack
     *****************
     * TODO: since its unreliable
     * to measure the amount of delimiters
     * and use that to determine
     * how many elements to take of the stack.
     ****
     * We will have to keep a counter,
     * expressions such as
     *   * inner groups,
     *   * if statements,
     *   * function defining
     * should count as 1 expressions, since
     * thats how they're represented in the stack
    ** this is essential to ensure code-blocks work
    */
    uint16_t expr_cnt;

    /*
    ** where the grouping is inside
    ** of the operator stack
    */
    uint16_t operator_idx;
    uint16_t set_idx;

    enum onk_lexicon_t type;
    bool is_empty;
    bool is_short;

    /* tear down many */
    bool collapse;
    //enum onk_lexicon_t stop_short[2];

    // should be `[` `(` '{' or `0`
    const struct onk_token_t *origin;
    const struct onk_token_t *last_delim;

    /*
    ** TODO: implement
    **       will cause UB -
    **       don't be a prick, warn the user.
    ** RULES:
    ** if (x) x;
    **         ^ After delimiter, check for `else`
    ** else y;
    **/
    //uint8_t short_block;
    //enum ShortBlock_t short_type;
};

/*
** restoration works by destroying a
** portion of the upper part of the stack.
**
** It will slice the top (newest) portion of the stack
** mark INCOMPLETE, and continue.
*/
struct onk_parser_snapshot_t {
    /* points to storation point  */
    const struct onk_token_t * operator_stack_tok;
    const struct onk_token_t * output_tok;
    const struct onk_token_t * current;
    const struct onk_parse_group_t * grp;
};


/* used to construct an error */
struct onk_partial_err_t {
    enum onk_parse_err_t type;
    struct onk_token_t start;

    enum onk_lexicon_t *expect;
    uint16_t nexpected;
};

struct onk_parser_state_t {
    const struct onk_token_t *src;
    const char * src_code;
    uint16_t src_sz;
    uint16_t *_i;

    /* a stack of pending operations (see shunting yard) */
    const struct onk_token_t *operator_stack[ONK_STACK_SZ];

    /* tracks opening braces in the operator stack */
    struct onk_parse_group_t set_stack[ONK_STACK_SZ];

    /* Acts as a stack, except when new groups are added
     * this stack is popped, and it will
     * define rules on item pushed
     * onto the set stack
     */
    //struct onk_parse_group_tQueue pending_group[STACK_SZ];

    /* tracks the previous groups we popped, where the most
     * recent group is at the top of the LONK_IF_TOKENO stack*/
    //struct Onk_Queue8 completed_groups;


    /* tracks opening braces in the operator stack */
    // struct onk_parse_group_t prev_set_stack[16];

    uint16_t set_ctr;
    uint16_t operators_ctr;

    uint16_t operator_stack_sz;
    uint16_t set_sz;

    /*
    ** Keep generated tokens in `pool`.
    ** Generated meaning they were not previously
    ** created in the previous stage (lexing)
    */
    /*Vec<struct onk_token_t>*/
    struct onk_vec_t pool;

    /* Vec<struct onk_token_t *> */
    struct onk_vec_t debug;

    /*Vec<struct ParseError>*/
    struct onk_vec_t errors;

    enum onk_lexicon_t *expect;
    uint16_t nexpect;
    uint16_t expect_capacity;

    /*Vec<struct onk_parser_snapshot_t>*/
    struct onk_vec_t restoration_stack;
    uint16_t restoration_ctr;


    /* whenever panic is set a
     * partial_err is valid, and
     * caused on previous loop */
    bool panic;
    struct onk_partial_err_t partial_err;

    uint16_t peek_next;
    uint16_t peek_prev;

    /* enabled if parser cannot
     * move to the next stage */
    bool stage_failed;
};


int8_t onk_parser_init(
  struct onk_parser_state_t*state,
  const struct onk_parser_input_t *in,
  uint16_t *i
);

int8_t onk_parser_free(struct onk_parser_state_t*state);
int8_t onk_parser_reset(struct onk_parser_state_t*state);
#endif
