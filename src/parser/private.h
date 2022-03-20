#ifndef __HEADER_AST_PRIV__
#define __HEADER_AST_PRIV__

#include <stdint.h>
#include <stdbool.h>

#include "lexer.h"
#include "parser.h"

/*
    The grouping stack is used to track the amount
    of sub-expressions inside an expression. (See lexer.h)
    We generate GROUPING tokens based on the stack model
    our parser uses.

    For every new brace token-type added into the operator-stack
    increment the grouping stack, and initalize it to 0.
    For every comma, increment the current grouping-stack's head by 1.

    Once the closing brace is found and
    this stack's head is larger than 0,
    we have a set/grouping of expressions.

    used in the group-stack exclusively
*/

struct Group {
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
struct RestorationFrame {
    /* points to storation point  */
    const struct onk_token_t * operator_stack_tok;
    const struct onk_token_t * output_tok;
    const struct onk_token_t * current;
    const struct Group * grp;
};


/* used to construct an error */
struct PartialError {
    enum ParserError_t type;
    struct onk_token_t start;

    enum onk_lexicon_t *expect;
    uint16_t nexpected;
};


struct PostfixStageState {

    /* Vec<struct Expr> */
    struct onk_vec_t pool;

    struct Expr * stack[ONK_STACK_SZ];
    uint16_t stack_ctr;
    uint16_t *_i;
};

struct Parser {
    const struct onk_token_t *src;
    const char * src_code;
    uint16_t src_sz;
    uint16_t *_i;

    /* a stack of pending operations (see shunting yard) */
    const struct onk_token_t *operator_stack[ONK_STACK_SZ];

    /* tracks opening braces in the operator stack */
    struct Group set_stack[ONK_STACK_SZ];

    /* Acts as a stack, except when new groups are added
     * this stack is popped, and it will
     * define rules on item pushed
     * onto the set stack
     */
    //struct GroupQueue pending_group[STACK_SZ];

    /* tracks the previous groups we popped, where the most
     * recent group is at the top of the LONK_IF_TOKENO stack*/
    //struct Onk_Queue8 completed_groups;


    /* tracks opening braces in the operator stack */
    // struct Group prev_set_stack[16];

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

    /*Vec<struct RestorationFrame>*/
    struct onk_vec_t restoration_stack;
    uint16_t restoration_ctr;


    /* whenever panic is set a
     * partial_err is valid, and
     * caused on previous loop */
    bool panic;
    struct PartialError partial_err;


    uint16_t peek_next;
    uint16_t peek_prev;

    /* enabled if parser cannot
     * move to the next stage */
    bool stage_failed;
};

void throw_unexpected_token(
  struct Parser *state,
  const struct onk_token_t *start,
  const enum onk_lexicon_t expected[],
  uint16_t nexpected
);

void restoration_hook(struct Parser *state);

int8_t handle_unwind(
    struct Parser *state,
    bool unexpected_token
);

/* used to construct an error */
/* struct PartialError { */
/*     enum ParserError_t type; */
/*     struct onk_token_t start; */

/*     enum onk_lexicon_t *expect; */
/*     uint16_t nexpected; */
/* }; */

//int8_t parser_free(struct Parser *state);
//int8_t parser_reset(struct Parser *state);

void insert(struct Parser *state, const struct onk_token_t *tok);
const struct onk_token_t * new_token(struct Parser *state, struct onk_token_t *tok);

void insert_new(
  enum onk_lexicon_t type,
  uint16_t start,
  uint16_t end,
  struct Parser *state
);

const struct onk_token_t * current_token(const struct Parser *state);
const struct onk_token_t * prev_token(const struct Parser *state) ;
const struct onk_token_t * next_token(const struct Parser *state);
const struct onk_token_t * op_head(const struct Parser *state);
const struct onk_token_t * op_push(enum onk_lexicon_t op, uint16_t start, uint16_t end, struct Parser *state);
const struct onk_token_t * output_head(const struct Parser *state);
struct Group * group_head(struct Parser *state);
struct Group * new_grp(struct Parser *state, const struct onk_token_t * origin);

bool can_ignore_token(enum onk_lexicon_t tok);

uint16_t find_next(struct Parser *state);

/*create token in pool, and push to output*/
void push_output(
  struct Parser *state,
  enum onk_lexicon_t type,
  uint16_t argc
);

const struct onk_token_t * group_modifier(
  const struct Parser *state,
  const struct Group *group
);

int8_t flush_ops(struct Parser *state);

int8_t push_group(struct Parser *state, const struct Group *grp);

bool is_op_keyword(enum onk_lexicon_t token);
int8_t op_precedence(enum onk_lexicon_t token);

int8_t finish_idx_access(struct Parser *state);
void idx_infer_value(struct Parser *state);


// enum onk_lexicon_t grp_dbg_sym(enum Group_t type);

int8_t push_many_ops(
  const enum onk_lexicon_t *ops,
  const struct onk_token_t *origin,
  struct Parser *state,
  uint16_t nitems
);


int8_t is_short_blockable(enum onk_lexicon_t tok);
bool onk_is_tok_unit(enum onk_lexicon_t tok);

enum Operation operation_from_token(enum onk_lexicon_t t);
//int8_t mk_error(struct Parser *state, enum ErrorT type, const char * msg);


bool is_fncall_pattern(enum onk_lexicon_t prev);
bool is_index_pattern(enum onk_lexicon_t prev);

int8_t init_parser(
  struct Parser *state,
  const struct ParserInput *in,
  uint16_t *i
);

int8_t parser_free(struct Parser *state);
int8_t parser_reset(struct Parser *state);

#endif
