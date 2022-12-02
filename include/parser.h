#ifndef __ONK_PARSER_HEADER__
#define __ONK_PARSER_HEADER__
#include "lexer.h"
#include <stdint.h>


struct onk_parser_output_t {
    /* Vec<struct onk_token_t *> */
    struct onk_vec_t postfix;

    /* Vec<struct onk_token_t> */
    struct onk_vec_t token_pool;

    /* Vec<struct ParseError> */
    struct onk_vec_t errors;

    bool stage_failed;
};

void onk_parser_output_free(struct onk_parser_output_t *);
enum onk_parse_err_t {
    parse_err_unexpected_token
};

struct UnexpectedTokError {
    /* onk_lexicon_t[N] null-terminated malloc */
    enum onk_lexicon_t *expected;
    uint16_t nexpected;

    struct onk_token_t thrown;
};

struct onk_parser_err_t {
    enum onk_parse_err_t type;

    /* window of tokens effected */
    struct onk_token_selection_t window;

    union {
        struct UnexpectedTokError unexpected_tok;

    } error;
};

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

#define _ONK_VALIDATOR_REF_SZ 8
#define _ONK_SEM_CHK_SZ 200

/*
define current_token(state) (state->src ? &state->src[state->_i] : panic())
define op_head(state) (state.op ? &state->src[state->_i] : panic())
*/

enum onk_parser_state_t_mode {
    onk_parser_uninit_mode = 0,
    onk_parser_exfil_mode = 1,
    onk_parser_init_mode = 2,
    onk_parser_alloc_mode = 3,
    onk_parser_awaiting_input_mode = 4,
    onk_parser_ready_mode = 5,
    onk_parser_free_mode = 6
};

struct onk_parser_state_t {
    struct onk_token_t *tokens;
    enum onk_parser_state_t_mode _mode;

    uint16_t token_len;
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

    /* Vec<struct ParseError> */
    struct onk_vec_t errors;

    enum onk_lexicon_t expect[_ONK_SEM_CHK_SZ];
    enum onk_lexicon_t  * exp_slices_buf[_ONK_VALIDATOR_REF_SZ];

    uint16_t islices_buf[_ONK_VALIDATOR_REF_SZ];
    uint16_t nexpect;

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

struct onk_parser_input_t {
    struct onk_vec_t tokens;
    bool add_glob_scope;
};

void onk_parser_input_free(struct onk_parser_input_t *);
void onk_parser_init(struct onk_parser_state_t *state, uint16_t *i);
void onk_parser_alloc(struct onk_parser_state_t *state);

enum onk_parse_loopctl_t {
  onk_parse_loopctl_continue,
  onk_parse_loopctl_finished,
  onk_parse_loopctl_panic,
};

void onk_parser_setup(struct onk_parser_state_t *state);

void onk_parser_setup_input(
    struct onk_parser_state_t *state,
    const struct onk_parser_input_t *input
);

void onk_parser_construct(
    struct onk_parser_state_t *parser,
    uint16_t *i,
    struct onk_parser_input_t *input
);

// unconditionally free parser members
void onk_parser_free(struct onk_parser_state_t *state);

// check before free
void onk_parser_sfree(struct onk_parser_state_t *state);

int8_t onk_parser_reset(struct onk_parser_state_t*state);

void onk_parser_input_from_lexer_output(
    const struct onk_lexer_output_t *lex,
    struct onk_parser_input_t *parser_in,
    bool add_glob_scope
);

// move vectors out of parser state
void onk_parser_output_from_parser_state(
    struct onk_parser_output_t *out,
    struct onk_parser_state_t *state
);

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

   This function implements a "fancy" shunting yard parser.
   returning a vec of pointers, of the source tokens in postfix notation.

   "Fancy" means to include extra features that will be described below.
   These include the ulities needed to flourish the shunting yard
   algorithm into a full parser.

   prerequisite know-how:
     shunting yard parses infix arithmitic notation,
     and converts it into postfix arithmitic nottation.
     Infix notation: "1 + 2 + 3"
     postfix notation: "1 2 + 3 +"
     postfix notation is much easier to compute using a stack.

   Fancy features:
    ## Grouping:
       Grouping occurs when a collection of units or expressions is
       surrounded in braces and delimated by group's type delimiter.
       Grouping creates its own tokens where the token type is the following

       ```
       onk_tuple_group_token,
       onk_struct_group_token,
       onk_list_group_token,
       onk_idx_group_token,
       onk_map_group_token,
       onk_code_group_token
       ```
       groups store the amount of members in `struct onk_token_t->end`
       Groups can be defined as empty by marking `end` as 0.

       Example input: [1, 2, 3]; ()
       Example output: 1 2 3 onk_list_group_token(end=3) onk_tuple_group_token(end=0)

       # Tuple group
       Example: (a, b)
       Example output: a b onk_tuple_group_token(2)
       delimiter: ','
       Tuples are immutable collections of data

       # onk_struct_group_token
       Example: Name {a = b}
       Output: Name (A b =) onk_struct_group_token(1) onk_struct_init_op_token(operator)
       delimiter: ','
       Structures define a new type of data,
       composed of other types.

       # onk_list_group_token
       Example: [a, b]
       Output: a b onk_list_group_token(2)
       delimiter: ','

       dynamically sized array

       # onk_idx_group_token
       Example: foo[1:2:3]
       Output: foo (1 2 3 onk_idx_group_token(3)) IndexAccess(operator)
       delimiter: ':'

       Creates an index selection
       on the previous expression.
       The numbers wrapped in braces represent the following
       parameters.

       ACCESSED [START:END:SKIP]

       The value in these parameters may be skipped,
       and the value will be inferred from min/max
       value based on the parameter.
       START - min(0)
       END - max(N items)
       skip - min(0)

       skipped argument example: foo[::2]

       # onk_map_group_token
       Example: {'a': 2, 'foo': 'bar'}
       Output: 'a' 2 'foo' 'bar' onk_map_group_token(4),
       Delimiter: ':' & ','

       HashMap are collections of keys, and data
       where each key is unique

       # Code block
       Example: { foo(); bar(); }
       Output: foo() bar() onk_code_group_token(2)
       Delimiter: ';'
       A collection of proceedures

      ## Extended Operator:
      ## `ONK_DOT_TOKEN` operator

      `ONK_DOT_TOKEN` is used to access properities of its parent structure.
      Inside the parser, `ONK_DOT_TOKEN` is treated as if its an binary operation.

      Exmaple input: abc.foo.last;
      Example output: abc foo . last .

      ## `onk_apply_op_token` operator
      `onk_apply_op_token` is used to call function
*/


int8_t onk_parse_steps(struct onk_parser_state_t *state);

int8_t onk_rc_parse(
    struct onk_parser_input_t *input,
    struct onk_parser_output_t *out
);

struct onk_parser_output_t onk_parse(
    struct onk_parser_input_t *input
);

#endif
