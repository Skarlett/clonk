#ifndef __ONK_PRIVATE_VALIDATOR__
#define __ONK_PRIVATE_VALIDATOR__

#include "clonk.h"
#include "lexer.h"

bool kw_follows_open_param(enum onk_lexicon_t tok);

bool follows_word(enum onk_lexicon_t tok);

bool can_use_else(enum onk_lexicon_t output_head);

/*
 * Can use Keywords if operator stack is flushed
 * and the top frame is a codeblock
*/
bool can_use_keywords(struct Parser *state);

#endif
