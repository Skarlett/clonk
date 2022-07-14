#include "clonk.h"
#include "lexer.h"
#include "parser.h"
#include "private.h"
#include "predict.h"

const enum onk_lexicon_t EXPR[] = {_EX_EXPR};
const enum onk_lexicon_t KWORD_BLOCK[KWORD_BLOCK_LEN] = {_EX_KWORD_BLOCK};
const enum onk_lexicon_t JMP_OPEN_PARAM[] = {};


/* start of static data structure */
#define ROWS 4
const enum onk_lexicon_t CATCH_GRP_A[] = {ONK_WORD_TOKEN, ONK_BRACKET_CLOSE_TOKEN, ONK_PARAM_CLOSE_TOKEN};
const enum onk_lexicon_t CATCH_GRP_B[] = {ONK_STRING_LITERAL_TOKEN};
const enum onk_lexicon_t CATCH_GRP_C[] = {ONK_BRACE_CLOSE_TOKEN};
const enum onk_lexicon_t CATCH_GRP_D[] = {ONK_TRUE_TOKEN, ONK_FALSE_TOKEN, ONK_INTEGER_TOKEN};

const enum onk_lexicon_t * CATCH[] = {
    CATCH_GRP_A,
    CATCH_GRP_B,
    CATCH_GRP_C,
    CATCH_GRP_D
};

const uint8_t ICATCH[] = {
  3,
  1,
  1,
  3
};

const enum onk_lexicon_t SET_GRP_A[] = {_EX_ASN_OPERATOR};
const enum onk_lexicon_t SET_GRP_B[] = {ONK_DOT_TOKEN};
const enum onk_lexicon_t SET_GRP_C[] = {ONK_BRACKET_OPEN_TOKEN, ONK_PARAM_OPEN_TOKEN};
const enum onk_lexicon_t SET_GRP_D[] = {_EX_BIN_OPERATOR};
const enum onk_lexicon_t * SET[] = {
    SET_GRP_A,
    SET_GRP_B,
    SET_GRP_C,
    SET_GRP_D
};

const uint8_t ISET[] = {
  ASNOP_LEN,
  1,
  2,
  BINOP_LEN
};

// expects unit
int8_t get_precedence_expr(enum onk_lexicon_t unit)
{

    for(uint8_t i=0; ROWS > i; i++)
        for(uint8_t j=0; ICATCH[i] > j; j++)
            if(CATCH[i][j] == unit)
                return i;
    return -1;
}

uint8_t default_mode(
    struct validator_frame_t *f,
    struct onk_parser_state_t *state,
    enum onk_lexicon_t unit
){
    for(int8_t i=get_precedence_expr(unit);
        ROWS > i;
        i++
    ) add_slice(f, SET[i], ISET[i]);
}

/*
** deep clone data into *arr
*/

bool _onk_semantic_check(
  struct onk_parser_state_t *state,
  enum onk_lexicon_t current
){
  assert(state->nexpect > 0);

  for(uint16_t i = 0; state->nexpect > i; i++)
    if(state->expect[i] == current)
      return false;

  return true;
}


uint16_t _onk_semantic_compile(
  enum onk_lexicon_t *arr,
  uint16_t arr_sz,
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

bool rm_delim_terminal(enum onk_lexicon_t gmod) {
  return gmod == onk_ifcond_op_token
      || gmod == onk_while_cond_op_token;
}

/* `if/while` expects an explicit onk_open_param_token after its occurance */
int8_t explicit_open_param(enum onk_lexicon_t current)
{
  return current == ONK_IF_TOKEN
      || current == ONK_WHILE_TOKEN
      || current == ONK_FOR_TOKEN;
}

#define EXPLICIT_WORD_LEN 5
const enum onk_lexicon_t EXPLICIT_WORD[] = {
    ONK_STRUCT_TOKEN,
    ONK_IMPL_TOKEN,
    ONK_DEF_TOKEN,
    ONK_IMPORT_TOKEN,
    ONK_DOT_TOKEN
};

bool explicit_word(enum onk_lexicon_t current)
{
    return onk_lexarr_contains(
        current,
        (void*)EXPLICIT_WORD,
        EXPLICIT_WORD_LEN
    );
}

#define EXPLICIT_EXPR_LEN (3 + BINOP_LEN + ASNOP_LEN + UNIOP_LEN, DELIM_LEN)
const enum onk_lexicon_t EXPLICIT_EXPR[] = {
    ONK_BRACKET_OPEN_TOKEN,
    ONK_PARAM_OPEN_TOKEN,
    ONK_HASHMAP_LITERAL_START_TOKEN,
    _EX_BIN_OPERATOR,
    _EX_UNARY_OPERATOR,
    _EX_ASN_OPERATOR,
    _EX_DELIM,
};

bool explicit_expr(enum onk_lexicon_t current)
{
    return onk_lexarr_contains(
        current,
        (void*)EXPLICIT_EXPR,
        EXPLICIT_EXPR_LEN
    );
}

bool use_parameter_mode(enum onk_lexicon_t gmod)
{
  return gmod == DefSign
    || gmod == ONK_STRUCT_TOKEN
    || gmod == onk_struct_init_op_token;
}

bool is_inside_impl_body(
  enum onk_lexicon_t ophead,
  enum onk_lexicon_t gmod,
  enum onk_lexicon_t current
){
  return ophead == ONK_BRACE_OPEN_TOKEN
          && gmod == ONK_IMPL_TOKEN
          && (current == ONK_BRACE_OPEN_TOKEN
              || current == ONK_BRACE_CLOSE_TOKEN);
}

bool is_inside_for_args(
  enum onk_lexicon_t ophead,
  enum onk_lexicon_t gmod)
{

  return ophead == ONK_PARAM_OPEN_TOKEN
    && gmod == onk_for_args_op_token;
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


int8_t build_next_frame(struct onk_parser_state_t *state)
{
  const struct onk_token_t *current = current_token(state);
  const struct onk_token_t *ophead = op_head(state);
  struct onk_parse_group_t *ghead = group_head(state);
  enum onk_lexicon_t gmod = group_modifier(state, ghead);
  struct validator_frame_t frame;

  frame.slices = state->exp_slices_buf;
  frame.islices = state->islices_buf;
  frame.nslices = 0;
  frame.delim = 0;
  frame.brace = 0;

  if(gmod)
  {

      // def foo(x, y)
      //         ^
      if(gmod == DefSign)
      {
          frame.brace = onk_invert_brace(gmod);
          frame.delim = ONK_COMMA_TOKEN;
          switch(current->type)
          {

              case ONK_WORD_TOKEN:
                state->expect[0] =
                break;

          }
      }

      else if(gmod == ONK_STRUCT_TOKEN)
      {
          frame.brace = onk_invert_brace(gmod);
          frame.delim = ONK_COMMA_TOKEN;
          switch(current->type)
          {
              /* entry point */
              case ONK_OPEN_PARAM_TOKEN:
                break;


          }
      }

      // aaa { x=a, y=b, z=c };
      else if(gmod == onk_struct_init_op_token)
      {

      }

      // aaa { x=a, y=b, z=c };
      else if(gmod == onk_struct_init_op_token)
      {
      }

      // for(x, y) in X
      else if (gmod == onk_for_args_op_token)
      {
/* */
        switch(current->type) {
           case ONK_WORD_TOKEN;

        }
        {
            state->expect[0] = ;

            state->expect[1] = ONK_PARAM_CLOSE_TOKEN;
            validator->islices[0] = 1;
            validator->nslices = 1;
        }
        else if(current->type == ONK_COMMA_TOKEN)
        {
            state->expect[0] = ONK_WORD_TOKEN;
            state->nexpect = 1;
        };



      }

      else if(is_inside_impl_body(state))
      {
          state->expect[0] = ONK_DEF_TOKEN;
          state->nexpect = 1;
          return 0;
      }

      panic();
  }

  else if (explicit_block(current->type, ophead->type))
  {
      state->expect[0] = ONK_BRACE_OPEN_TOKEN;
      state->nexpect = 1;
      return 0;
  }

  else if (explicit_word(current->type))
  {
      state->expect[0] = ONK_WORD_TOKEN;
      state->nexpect = 1;
      return 0;
  }

  else if (explicit_open_param(current->type))
  {
      state->expect[0] = ONK_PARAM_OPEN_TOKEN;
      state->nexpect = 1;
      return 0;
  }


  // struct aaa { x, y, z=c }
  // def foo(x, y, z=c)


  else if(explicit_expr(current->type))
  {
    if(gmod == onk_idx_op_token && 2 > ghead->delimiter_cnt)
      frame.delim = delimiter(state);

    if(current->type != ONK_BRACKET_OPEN_TOKEN)
      frame.brace = ONK_BRACKET_CLOSE_TOKEN;


    add_slice(&frame, EXPR);
  }

  else panic();

  // ctx - if/while shouldn't contain delimitation
  if(rm_delim_terminal(frame, gmod))
  {

  }

  // ctx - if/while shouldn't contain immediate closing brace
  if(rm_brace_terminal(frame, gmod->type))
  {

  }

  // Add keywords
  if (ophead->type == ONK_BRACE_OPEN_TOKEN)
    add_slice(f, KWORD_BLOCK, KWORD_BLOCK_LEN);

  frame.brace = onk_invert_brace(ghead->origin->type);

  assert(frame.delim != 0);
  assert(frame.brace != 0);

  state->nexpect = _onk_semantic_compile(state->expect, ONK_PARSE_EXP_SZ, &frame);
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

