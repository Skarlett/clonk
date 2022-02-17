#ifndef __EXPR_UTILS__
#define __EXPR_UTILS__

#include <stdint.h>
#include <stdbool.h>

void insert(struct Parser *state, const struct Token *tok);
const struct Token * new_token(struct Parser *state, struct Token *tok);
void insert_new(
  enum onk_lexicon_t type,
  uint16_t start,
  uint16_t end,
  struct Parser *state
);

const struct Token * current_token(const struct Parser *state);
const struct Token * prev_token(const struct Parser *state) ;
const struct Token * next_token(const struct Parser *state);
const struct Token * op_head(const struct Parser *state);
const struct Token * op_push(enum onk_lexicon_t op, uint16_t start, uint16_t end, struct Parser *state);
const struct Token * output_head(const struct Parser *state);
struct Group * group_head(struct Parser *state);
struct Group * new_grp(struct Parser *state, const struct Token * origin);

/*create token in pool, and push to output*/
void push_output(
  struct Parser *state,
  enum onk_lexicon_t type,
  uint16_t argc
);

const struct Token * group_modifier(
  const struct Parser *state,
  const struct Group *group
);

int8_t flush_ops(struct Parser *state);

int8_t push_group(struct Parser *state, const struct Group *grp);

bool is_op_keyword(enum onk_lexicon_t token);
int8_t op_precedence(enum onk_lexicon_t token);

int8_t finish_idx_access(struct Parser *state);

// enum onk_lexicon_t grp_dbg_sym(enum Group_t type);

int8_t push_many_ops(
    const enum onk_lexicon_t *ops,
    const struct Token *origin,
    struct Parser *state
);


int8_t is_short_blockable(enum onk_lexicon_t tok);
bool is_unit(enum onk_lexicon_t tok);

enum Operation operation_from_token(enum onk_lexicon_t t);
//int8_t mk_error(struct Parser *state, enum ErrorT type, const char * msg);

int8_t throw_internal_error(struct Parser *state, const char * meta, const char * msg);

bool is_index_pattern(const struct Token *prev);

bool is_fncall_pattern(const struct Token *prev);

int8_t init_parser(
  struct Parser *state,
  const struct ParserInput *in,
  uint16_t *i
);

int8_t parser_free(struct Parser *state);
int8_t parser_reset(struct Parser *state);
#endif
