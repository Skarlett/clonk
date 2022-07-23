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

void default_mode(
    struct validator_frame_t *f,
    struct onk_parser_state_t *state,
    enum onk_lexicon_t unit
){
    for(int8_t i=get_precedence_expr(unit);
        ROWS > i;
        i++
    ) add_slice(f, SET[i], ISET[i]);
}

void init_frame(
    struct onk_parser_state_t *state,
    struct validator_frame_t *frame
){
    frame->slices = state->exp_slices_buf;
    frame->islices = state->islices_buf;
    frame->nslices = 0;
    frame->set_delim = true;
    frame->set_brace = true;
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

uint16_t onk_semantic_compile(
  struct onk_parser_state_t *state,
  struct validator_frame_t *frame
){

  uint16_t slice_sz = 0;
  uint16_t total = 0;
  onk_usize offset = 0;

  uint8_t slice_len = 0;
  uint16_t i = 0;

  for (i=0; frame->nslices > i; i++)
  {
    slice_len = frame->islices[i];
    slice_sz = sizeof(enum onk_lexicon_t) * slice_len;

    assert(_ONK_SEM_CHK_SZ > total + slice_len);

    assert(memcpy(
      (onk_usize)state->expect + total,
      frame->slices[i], slice_sz) > 0);

    total += slice_sz;
  }

  if (frame->set_delim)
    state->expect[total++] = place_delimiter(state);

  if (frame->set_brace)
    state->expect[total++] = onk_invert_brace(ghead->origin->type);

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
  return gmod == onk_defsig_op_token
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


// x {y=expr, z=expr}
int8_t struct_init_mode(
  struct validator_frame_t *frame,
  struct onk_parser_state_t *state
){
    enum onk_lexicon_t current = current_token(state)->type;

    // not actually in context
    /* if (ophead->type != expected_ophead_brace) */
    /*   return 2; */

    switch(current)
    {
        // a {..}
        //   ^
        case ONK_BRACE_OPEN_TOKEN:
            state->expect[0] = ONK_WORD_TOKEN;
            state->expect[1] = ONK_BRACE_CLOSE_TOKEN;
            state->nexpect = 2;
            return 1;

        // a { a = ...}
        //     ^->
        case ONK_WORD_TOKEN:
            state->expect[0] = ONK_EQUAL_TOKEN;
            state->nexpect = 1;
            return 1;

        // { a = <expr>}
        //     ^ ->
        case ONK_EQUAL_TOKEN:
            frame->slices[0] = (void*)EXPR;
            frame->islices[0] = EXPR_LEN;
            frame->nslices = 1;
            return 0;

        //
        // a { a = x, ..}
        //          ^->
        case ONK_COMMA_TOKEN:
            state->expect[0] = ONK_WORD_TOKEN;
            state->nexpect = 1;
            return 1;

        default:
            return -1;
    }
}

// x(y, z<?=expr>)
//   ^->
int8_t parameter_mode(
  struct validator_frame_t *frame,
  struct onk_parser_state_t *state
){
    enum onk_lexicon_t current = current_token(state)->type;
    switch(current)
    {

      // x(y, z<?=expr>)
      //   ^->
      case ONK_WORD_TOKEN:
        state->expect[0] = ONK_COMMA_TOKEN;
        state->expect[1] = ONK_PARAM_CLOSE_TOKEN;
        state->expect[2] = ONK_EQUAL_TOKEN;
        state->nexpect = 2;
        return 1;

      // x(y, z<?=expr>)
      //   ^->
      case ONK_COMMA_TOKEN:
        state->expect[0] = ONK_WORD_TOKEN;
        state->nexpect = 1;
        return 1;

      case ONK_EQUAL_TOKEN:
        add_slice(frame, EXPR, EXPR_LEN);
        return 0;

      default: return -1;
    }
}

int8_t build_next_frame(struct onk_parser_state_t *state)
{
  const struct onk_token_t *current = current_token(state);
  const struct onk_token_t *ophead = op_head(state);
  struct onk_parse_group_t *ghead = group_head(state);
  const struct onk_token_t *gmod = group_modifier(state, ghead);

  struct validator_frame_t frame;
  int8_t ctx_flag = 0;

  init_validator_frame(&frame, state);

  // def foo(x, y)
  //         ^
  if(gmod && ((gmod->type == onk_defsig_op_token && ophead->type == ONK_PARAM_OPEN_TOKEN)
      || (gmod->type == ONK_STRUCT_TOKEN && ophead->type == ONK_BRACE_OPEN_TOKEN)))
  {
      ctx_flag = parameter_mode(&frame, state);

      if(current->type == ONK_PARAM_OPEN_TOKEN || current->type == ONK_BRACE_OPEN_TOKEN)
      {
          state->expect[0] = onk_invert_brace(current->type);
          state->expect[1] = ONK_WORD_TOKEN;
          state->nexpect = 2;
          return 0;
      }

      switch(ctx_flag)
      {
        default: return -1;
        case 0: return 0;
        case 1:
          state->nexpect = onk_semantic_compile(
            state->expect,
            &frame
          );

          return 0;
      }
  }

  // aaa { x=a, y=b, z=c };
  else if(gmod && gmod->type == onk_struct_init_op_token
          && ophead->type == ONK_BRACE_OPEN_TOKEN)
  {
      switch(struct_init_mode(&frame, state))
      {
        default: return -1;
        case 0: return 0;
        case 1: state->nexpect = onk_semantic_compile(
            state->expect,
            &frame
          );

          return 0;
      }
  }

  // for(x, y) in X
  //    ^^
  else if (gmod && gmod->type == onk_for_args_op_token
           && ophead->type == ONK_PARAM_OPEN_TOKEN)
  {
        state->expect[0] = ONK_WORD_TOKEN;
        state->expect[1] = ONK_PARAM_CLOSE_TOKEN;
        state->nexpect = 2;
        return 0;
  }

  // impl word { def ... }
  //           ^-^
  else if(is_inside_impl_body(ophead->type, gmod->type, current->type))
  {
        state->expect[0] = ONK_DEF_TOKEN;
        state->nexpect = 1;
        return 0;
  }

  // for|if|while( ) { def ... }
  //               ^-^
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

  // if|for|while (
  //            ^-^
  else if (explicit_open_param(current->type))
  {
      state->expect[0] = ONK_PARAM_OPEN_TOKEN;
      state->nexpect = 1;
      return 0;
  }

  // a + x
  //   ^-^
  else if(explicit_expr(current->type))
  {
    /* if(gmod->type == onk_idx_op_token && 2 > ghead->delimiter_cnt) */
    /*   frame.delim = delimiter(state); */

    /* if(current->type != ONK_BRACKET_OPEN_TOKEN) */
    /*   frame.brace = ONK_BRACKET_CLOSE_TOKEN; */

    // add_slice(&frame, EXPR);
  }

  else { printf("reached unknown state"); exit(125); }

  // Add keywords
  if (ophead->type == ONK_BRACE_OPEN_TOKEN)
    add_slice(frame, KWORD_BLOCK, KWORD_BLOCK_LEN);


  state->nexpect = onk_semantic_compile(state->expect, &frame);
}

