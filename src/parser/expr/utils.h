#ifndef __EXPR_UTILS__
#define __EXPR_UTILS__

#include <stdint.h>
#include "../lexer/lexer.h"
#include "expr.h"

const struct Token * prev_token(struct Parser *state) ;
const struct Token * next_token(struct Parser *state);
const struct Token * op_head(struct Parser *state);
const struct Token * op_push(enum Lexicon op, uint16_t start, uint16_t end, struct Parser *state);
struct Group * group_head(struct Parser *state);
struct Group * new_grp(struct Parser *state, const struct Token * origin);

bool is_op_keyword(enum Lexicon token);
int8_t op_precedence(enum Lexicon token);

enum Lexicon grp_dbg_sym(enum GroupType type);

int8_t push_many_ops(
    const enum Lexicon *ops,
    const struct Token *origin,
    struct Parser *state
);


int8_t is_short_blockable(enum Lexicon tok);
bool is_unit_expr(enum Lexicon tok);

enum Operation operation_from_token(enum Lexicon t);
//int8_t mk_error(struct Parser *state, enum ErrorT type, const char * msg);

int8_t throw_internal_error(struct Parser *state, const char * meta, const char * msg);

#endif

