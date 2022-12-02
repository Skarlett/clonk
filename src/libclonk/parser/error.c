#include "clonk.h"
#include "lexer.h"
#include "parser.h"
#include "private.h"
#include "semantics.h"

/**
 * @param tok: comparsion token
 */
int8_t is_continuable(enum onk_lexicon_t tok) {
    static enum onk_lexicon_t __BREAK_POINTS_START[] = {
        _EX_BIN_OPERATOR,
        _EX_ASN_OPERATOR,
        _EX_CLOSE_BRACE,
        _EX_OPEN_BRACE,

        // TODO: Investigate this
        //_EX_DELIM,
        0
    };
    static const uint16_t len = BINOP_LEN + ASNOP_LEN \
        + BRACE_CLOSE_LEN + BRACE_OPEN_LEN; // + _EX_DELIM_LEN;

    return onk_lexarr_contains(tok, __BREAK_POINTS_START, len);
}


void throw_unexpected_token(
  struct onk_parser_state_t*state,
  const struct onk_token_t *start,
  const enum onk_lexicon_t expected[],
  uint16_t nexpected
){

    struct onk_partial_err_t *err = &state->partial_err;
    enum onk_lexicon_t *expect_heap;

    expect_heap = calloc(nexpected, sizeof(enum onk_lexicon_t));
    memcpy(expect_heap, expected, sizeof(enum onk_lexicon_t) * nexpected);

    err->type = parse_err_unexpected_token;
    err->start = *start;
    err->expect = expect_heap;
    err->nexpected = nexpected;

    state->panic=true;
}

/*
** Grab the last inserted frame
*/
const struct onk_parser_snapshot_t * restoration_head(const struct onk_parser_state_t*state) {
    return &((struct onk_parser_snapshot_t *)state->restoration_stack.base)[(state->restoration_ctr || 1) - 1];
}

/*
** Returns boolean if restoration hook should be ran
**
*/
bool run_hook(enum onk_lexicon_t current){
    return onk_is_tok_delimiter(current)
        || onk_is_tok_open_brace(current)
        || onk_is_tok_close_brace(current);
}


void restoration_hook(struct onk_parser_state_t*state)
{
    struct onk_parser_snapshot_t rframe;
    const struct onk_token_t *current = &state->tokens[*state->_i];

    if (run_hook(current->type))
    {
        /* pop last delimiter restore point  */
        if (onk_is_tok_delimiter(current->type))
            state->restoration_ctr -= 1;

        /* pop last delimiter & open brace */
        else if (onk_is_tok_close_brace(current->type))
            state->restoration_ctr -= 2;

        /* debug = Vec<struct onk_token_t*> */
        rframe.operator_stack_tok = state->operator_stack[state->operators_ctr];
        rframe.output_tok = onk_vec_head(&state->debug);
        rframe.current = &state->tokens[*state->_i];

        onk_vec_push(&state->restoration_stack, &rframe);
    }
}

uint16_t recover_cursor(
    struct onk_parser_state_t*state,
    bool preloop
) {
    uint16_t start = *state->_i;
    uint16_t continue_ctr = 0;

     // pop until output matches restoration
    if (preloop)
        start += 1;

     for (continue_ctr=start; state->token_len > continue_ctr; continue_ctr++)
     {
       if (is_continuable(state->tokens[continue_ctr].type))
         return continue_ctr;
     }

     return 0;
}

//TODO: watch out for tokens with `0`
//      marked as their sequence
void unwind_stacks(struct onk_parser_state_t*state)
{
    const struct onk_token_t *token;
    const struct onk_parser_snapshot_t *rframe;
    struct onk_parse_group_t *grp;
    uint16_t head = 0;

    /* clear everything */
    if (state->restoration_ctr == 0) {
        state->set_ctr = 0;
        state->operators_ctr = 0;
        onk_vec_clear(&state->debug);
        return;
    }

    token = onk_vec_head(&state->debug);
    
    /* get last good restoration */
    rframe = restoration_head(state);
    head = rframe->output_tok->seq;

    // pop until out matches restoration
    while (state->debug.len > 0 && token->seq > head)
    {
        onk_vec_pop(&state->debug, 0);
        token = onk_vec_head(&state->debug);
    }

    head = rframe->operator_stack_tok->seq;
    token = op_head(state);

    while(state->operators_ctr > 0 && token->seq > head)
    {
        token = op_head(state);
        state->operators_ctr -= 1;
    }

    head = rframe->grp->seq;
    grp = group_head(state);

    while(state->set_ctr > 0 && grp->seq > head)
    {
        grp = group_head(state);
        state->set_ctr -= 1;
    }

}

void mk_window_union(
    struct onk_parser_err_t *err,
    struct onk_token_t start,
    struct onk_token_t end
){
    err->window.type = Union;
    err->window.token.union_t.start = start;
    err->window.token.union_t.end = end;
}

void mk_window_scalar(
    struct onk_parser_err_t *err,
    struct onk_token_t token
){
    err->window.type = Scalar;
    err->window.token.scalar_t = token;
}

int8_t mk_window(
    struct onk_parser_err_t *err,
    struct onk_parser_state_t*state,
    const struct onk_token_t *start,
    uint16_t ctr)
{

    // TODO: clean up assert into proper error
    // No tokens found to continue from
    //
    assert(ctr != 0);

    if (ctr == state->token_len)
        return -1;

    else if (ctr - *state->_i > 1)
    {
        mk_window_union(err, *start, state->tokens[ctr - 1]);
        *state->_i = ctr;
    }
    else
    {
        mk_window_scalar(err, *start);
        *state->_i += 1;
    }

    return 0;
}



/*
  unwind the parser to a previous safe-state
  ----
  1) find break point.
  2) undo stack instrutions
  3) find break point past intrusion
  4) report error
  5) continue parsing

  TODO: when continuation token is found,
  create dumbie values to place in output.

*/
int8_t handle_unwind(
    struct onk_parser_state_t*state,
    bool preloop
){
    const struct onk_token_t *start;
    struct onk_partial_err_t *perr;
    struct onk_parser_err_t err;

    struct UnexpectedTokError unexpected_tok;
    uint16_t continue_ctr = 0;

    err.type = parse_err_unexpected_token;

    /* all other errors complete the main
    ** loop before panicing
    */
    start = &state->tokens[*state->_i - 1];
    if (preloop)
        start = &state->tokens[*state->_i];


    continue_ctr = recover_cursor(state, preloop);

    unwind_stacks(state);
    mk_window(&err, state, start, continue_ctr);

    err.error.unexpected_tok = unexpected_tok;
    onk_vec_push(&state->errors, &err);
    return 0;
}

int8_t handle_error(
    struct onk_parser_state_t *state,
    struct onk_parser_err_t *err,
    /* signifies if error
     * happened on the previous loop
     * In the case that this is false,
     * Its an unexpected token
     * foreseen by previsioning stage
    */
    bool prev_loop
) {

    //handle_unwind()
    return 0;

}
