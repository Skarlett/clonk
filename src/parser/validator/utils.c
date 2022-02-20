#include "clonk.h"
#include "lexer.h"
#include "predict.h"

bool kw_follows_open_param(enum onk_lexicon_t tok)
{
  return tok == ONK_IF_TOKEN || tok == ONK_WHILE_TOKEN;
}


bool follows_word(enum onk_lexicon_t tok)
{
  return (tok == ONK_STRUCT_TOKEN
    || tok == ONK_DEF_TOKEN
    || tok == ONK_IMPL_TOKEN
  );
}

bool can_use_else(enum onk_lexicon_t output_head){
  return output_head == onk_ifbody_op_token;
}

/*
 * Can use Keywords if operator stack is flushed
 * and the top frame is a codeblock
*/
bool can_use_keywords(struct Parser *state)
{
  struct Group *ghead = group_head(state);
  const struct onk_token_t *ophead = op_head(state);

  return (ophead->type == ONK_BRACE_OPEN_TOKEN
          && (ghead->type == onk_partial_brace_group_token
              || ghead->type == onk_code_group_token));

}
