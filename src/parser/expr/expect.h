#ifndef _HEADER__EXPECT__
#define _HEADER__EXPECT__

#include <stdint.h>
#include <sys/types.h>
#include "../lexer/lexer.h"
#include "expr.h"

enum ExpectMode {
  Default,
  DefSig
};

struct ExpectState {
  enum Lexicon exp[64];
  enum Lexicon *exp_ref;
  enum ExpectMode mode;
};

int8_t is_token_unexpected(struct ExprParserState *state);

void init_expect_buffer(struct ExprParserState *state);

/* void unset_flag(FLAG_T *set, FLAG_T flag); */
/* void set_flag(FLAG_T *set, FLAG_T flag); */
/* FLAG_T check_flag(FLAG_T set, FLAG_T flag); */

#endif

