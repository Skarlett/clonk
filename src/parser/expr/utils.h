#ifndef __EXPR_UTILS__
#define __EXPR_UTILS__

#include <stdint.h>
#include "../lexer/lexer.h"
#include "expr.h"

int8_t add_dbg_sym(
  struct ExprParserState *state,
  enum Lexicon type,
  uint16_t argc
);

int8_t inc_stack(
  struct ExprParserState *state,
  struct Expr *ex,
  struct Token *dbg_out
);

struct Token * prev_token(struct ExprParserState *state) ;
struct Token * next_token(struct ExprParserState *state);
struct Token * op_head(struct ExprParserState *state);
struct Token * op_push(enum Lexicon op, uint16_t start, uint16_t end, struct ExprParserState *state);
struct Group * group_head(struct ExprParserState *state);
struct Group * new_grp(struct ExprParserState *state, struct Token * origin);

bool is_op_keyword(enum Lexicon token);
int8_t op_precedence(enum Lexicon token);

enum Lexicon grp_dbg_sym(enum GroupT type);

int8_t push_many_ops(enum Lexicon *ops, struct Token *origin, struct ExprParserState *state);
int8_t is_short_blockable(enum Lexicon tok);
bool is_unit_expr(enum Lexicon tok);

int8_t mk_error(struct ExprParserState *state, enum ErrorT type, const char * msg);

int8_t throw_internal_error(struct ExprParserState *state, const char * meta, const char * msg);

#endif

