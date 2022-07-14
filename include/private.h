#ifndef __HEADER_AST_PRIV__
#define __HEADER_AST_PRIV__

#include <stdint.h>
#include "clonk.h"

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

void throw_unexpected_token(
  struct onk_parser_state_t *state,
  const struct onk_token_t *start,
  const enum onk_lexicon_t expected[],
  uint16_t nexpected
);

void restoration_hook(struct onk_parser_state_t *state);

int8_t handle_unwind(
    struct onk_parser_state_t*state,
    bool unexpected_token
);


//int8_t parser_free(struct onk_parser_state_t*state);
//int8_t parser_reset(struct onk_parser_state_t*state);

void insert(struct onk_parser_state_t*state, const struct onk_token_t *tok);
const struct onk_token_t * new_token(struct onk_parser_state_t*state, struct onk_token_t *tok);

void insert_new(
  enum onk_lexicon_t type,
  uint16_t start,
  uint16_t end,
  struct onk_parser_state_t*state
);

const struct onk_token_t * current_token(struct onk_parser_state_t*state);
const struct onk_token_t * prev_token(const struct onk_parser_state_t*state) ;
const struct onk_token_t * next_token(const struct onk_parser_state_t*state);
const struct onk_token_t * op_head(const struct onk_parser_state_t*state);
const struct onk_token_t * op_push(enum onk_lexicon_t op, uint16_t start, uint16_t end, struct onk_parser_state_t*state);
const struct onk_token_t * output_head(const struct onk_parser_state_t*state);
struct onk_parse_group_t * group_head(struct onk_parser_state_t*state);
struct onk_parse_group_t * new_grp(struct onk_parser_state_t*state, const struct onk_token_t * origin);

bool can_ignore_token(enum onk_lexicon_t tok);

uint16_t find_next(struct onk_parser_state_t*state);

/*create token in pool, and push to output*/
void push_output(
  struct onk_parser_state_t*state,
  enum onk_lexicon_t type,
  uint16_t argc
);

const struct onk_token_t * group_modifier(
  const struct onk_parser_state_t*state,
  const struct onk_parse_group_t *group
);

int8_t flush_ops(struct onk_parser_state_t*state);

int8_t push_group(struct onk_parser_state_t*state, const struct onk_parse_group_t *grp);

bool is_op_keyword(enum onk_lexicon_t token);
int8_t op_precedence(enum onk_lexicon_t token);

int8_t finish_idx_access(struct onk_parser_state_t*state);
void idx_infer_value(struct onk_parser_state_t*state);

void init_grp(struct onk_parse_group_t * ghead, enum onk_lexicon_t from);

// enum onk_lexicon_t grp_dbg_sym(enum Group_t type);

int8_t push_many_ops(
  const enum onk_lexicon_t *ops,
  const struct onk_token_t *origin,
  struct onk_parser_state_t*state,
  uint16_t nitems
);


int8_t is_short_blockable(enum onk_lexicon_t tok);
bool onk_is_tok_unit(enum onk_lexicon_t tok);

enum Operation operation_from_token(enum onk_lexicon_t t);
//int8_t mk_error(struct onk_parser_state_t*state, enum ErrorT type, const char * msg);


bool is_fncall_pattern(enum onk_lexicon_t prev);
bool is_index_pattern(enum onk_lexicon_t prev);
#endif
