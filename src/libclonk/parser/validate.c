#include "lexer.h"
#include "parser.h"
#include "private.h"
#include "predict.h"

#define JMP_EXPR_LEN (BRACE_OPEN_LEN + BINOP_LEN + ASNOP_LEN + UNIOP_LEN)
const enum onk_lexicon_t JMP_EXPR[] = {
    _EX_OPEN_BRACE,
    _EX_BIN_OPERATOR,
    _EX_ASN_OPERATOR,
    _EX_UNARY_OPERATOR,
};

const enum onk_lexicon_t EXPR[] = {_EX_EXPR};
const enum onk_lexicon_t KWORD_BLOCK[KWORD_BLOCK_LEN] = {_EX_KWORD_BLOCK};

const enum onk_lexicon_t JMP_OPEN_PARAM[] = {};

const enum onk_lexicon_t CATCH_GRP_A[] = {ONK_WORD_TOKEN};
const enum onk_lexicon_t CATCH_GRP_B[] = {ONK_BRACKET_CLOSE_TOKEN, ONK_PARAM_CLOSE_TOKEN};
const enum onk_lexicon_t CATCH_GRP_C[] = {ONK_STRING_LITERAL_TOKEN, ONK_BRACE_CLOSE_TOKEN};
const enum onk_lexicon_t CATCH_GRP_D[] = {ONK_TRUE_TOKEN, ONK_FALSE_TOKEN, ONK_INTEGER_TOKEN};

const enum onk_lexicon_t * CATCH[] = {
    CATCH_GRP_A,
    CATCH_GRP_B,
    CATCH_GRP_C,
    CATCH_GRP_D
};

const uint8_t ICATCH[] = {
  1,
  2,
  2,
  3
};

const enum onk_lexicon_t SET_GRP_A[] = {ONK_BRACE_OPEN_TOKEN};
const enum onk_lexicon_t SET_GRP_B[] = {_EX_ASN_OPERATOR};
const enum onk_lexicon_t SET_GRP_C[] = {ONK_BRACKET_OPEN_TOKEN, ONK_PARAM_OPEN_TOKEN, ONK_DOT_TOKEN};
const enum onk_lexicon_t SET_GRP_D[] = {_EX_BIN_OPERATOR};
const enum onk_lexicon_t * SET[] = {
    SET_GRP_A,
    SET_GRP_B,
    SET_GRP_C,
    SET_GRP_D
};

const uint8_t ISET[] = {
  1,
  ASNOP_LEN,
  3,
  BINOP_LEN
};

struct record_t {
  const enum onk_lexicon_t **catch;
  const enum onk_lexicon_t **set;

  const uint8_t *iset;
  const uint8_t *icatch;

  const uint8_t nset;
};

const struct record_t MAP = {
  .catch=CATCH,
  .set=SET,
  .iset=ISET,
  .icatch=ICATCH,
  .nset=4
};

enum onk_lexicon_t place_closing_brace(struct onk_parser_state_t*state)
{
  struct onk_parse_group_t *ghead = group_head(state);

  /* const struct onk_token_t *current = current_token(state); */
  /* const struct onk_token_t *gmod = group_modifier(state, ghead); */

  /* if(current->type == ONK_PARAM_OPEN_TOKEN) */
  /* { */
  /*   if(gmod->type == onk_ifcond_op_token */
  /*     || gmod->type == onk_while_cond_op_token) */
  /*     return ONK_UNDEFINED_TOKEN; */
  /* } */

  /* else if (current->type == ONK_BRACKET_OPEN_TOKEN */
  /*          && ghead->type == onk_idx_group_token) */
  /*          return ONK_UNDEFINED_TOKEN; */

  return onk_invert_brace(ghead->origin->type);
}


// expects unit
int8_t get_precedence_expr(enum onk_lexicon_t unit)
{
    for(uint8_t i=0; MAP.nset > i; i++)
        for(uint8_t j=0; MAP.icatch[i] > j; j++)
            if(MAP.catch[i][j] == unit)
                return i;
    return -1;
}

uint8_t default_mode(
    struct validator_frame_t *f,
    struct onk_parser_state_t *state,
    enum onk_lexicon_t unit
){
    /* const struct onk_token_t *ophead = op_head(state); */
    /* struct onk_parse_group_t *ghead = group_head(state); */

    /* if(onk_lexarr_contains(unit, JMP_EXPR, JMP_EXPR_LEN)) */
    /*     add_slice(f, EXPR, EXPR_LEN); */

    for(int8_t i=get_precedence_expr(unit);
        i > 0;
        --i
    ) add_slice(f, SET[i], ISET[i]);

    // Add keywords
    /* if (ophead->type == ONK_BRACE_OPEN_TOKEN) */
    /*     add_slice(f, KWORD_BLOCK, KWORD_BLOCK_LEN); */

    /* f->delim_terminal = place_delimiter(state); */
    /* f->brace_terminal = onk_invert_brace(ghead->origin->type); */

    assert(f->delim_terminal != 0);
    assert(f->brace_terminal != 0);



    
}

/*
** deep clone data into *arr
*/
uint16_t onk_semantic_compile(
  enum onk_lexicon_t *arr,
  struct validator_frame_t *validator
)
{
  uint16_t slice_sz = 0;
  uint16_t total = 0;
  onk_usize offset = 0;

  uint8_t slice_len = 0;
  uint16_t i = 0;

  for (i=0; validator->nslices > i; i++)
  {
    slice_len = validator->islices[i];
    slice_sz = sizeof(enum onk_lexicon_t) * slice_len;

    assert(total + slice_len > arr_sz);

    total += slice_len;

    assert(memcpy(
      (onk_usize)arr + offset,
      validator->slices[i], slice_sz) > 0);

    offset += slice_sz;
  }

  return total;
}

bool rm_delim_terminal(struct validator_frame_t *frame, gmod) {
  return gmod == onk_ifcond_op_token || gmod == onk_while_cond_op_token;
}


/* use `{` next */
bool explicit_block(
  enum onk_lexicon_t current,
  enum onk_lexicon_t ophead
){
  return
    /*   def sig() { */
    /* while sig() { */
    /*  if(foobar) { */
    /*           ^-^ */
    ((ophead == onk_defbody_op_token
      || ophead == onk_while_body_op_token
      || ophead == onk_ifbody_op_token)
      && current == ONK_PARAM_CLOSE_TOKEN)

    /* struct/impl word { */
    /*                ^-^ */
    ||((ophead == ONK_STRUCT_TOKEN
        || ophead == ONK_IMPL_TOKEN)
        && current == ONK_WORD_TOKEN)

    /* for x in expr { */
    /*             ^-^ */
    || (ophead == onk_for_body_op_token
        && onk_is_tok_unit(current));
}


void build_next_frame(struct onk_parser_state_t *state)
{
  struct validator_frame_t frame;

  frame.slices = state->exp_slices_buf;
  frame.islices = state->islices_buf;
  frame.nslices = 0;

  frame.delim_terminal = 0;
  frame.brace_terminal = 0;
  
  if (explicit_block(current, ophead)) {
    state->expect[0] = ONK_BRACE_OPEN_TOKEN;
    state->nexpect = 1;
    return;
  }

  else if (explicit_word(current, ophead)) {
    state->expect[0] = ONK_WORD_TOKEN;
    state->nexpect = 1;
    return;
  }

  else if ()
  {

    
    // add_operators();

  }

  else if (is_onk_bin_operator)

  if(gmod != 0)
  {
    check_ctx()
    return false;
  }

  // ctx - if/while shouldn't contain delimitation
  if rm_delim_terminal(frame, gmod->type)
  {

  }

  // ctx - if/while shouldn't contain immediate closing brace
  if rm_brace_terminal(frame, gmod->type)
  {

  }


  state->nexpect = onk_semantic_compile(state->expect, &frame);
}


bool is_token_unexpected(struct onk_parser_state_t *state)
{
  enum onk_lexicon_t current = current_token(state)->type;
  struct onk_parse_group_t *ghead = group_head(state);
  struct onk_token_t *ophead = op_head(state);
  struct onk_parse_group_t *gmod = group_modifier(state, ghead);

  assert(state->nexpect > 0);

  if (!onk_lexarr_contains(current, state->expect, state->nexpect))
      return true;

  build_next_frame();
  return false;
}

void (
    struct validator_frame_t *f,
    struct onk_parser_state_t *state,
    enum onk_lexicon_t unit
)
{
    if
}
