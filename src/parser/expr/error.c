#include "expr.h"
#include <string.h>
#include "../../error.h"
#include <assert.h>
#include <stdint.h>

/**
 * @param tok: comparsion token
 */
int8_t is_continuable(enum Lexicon tok) {
    static enum Lexicon __BREAK_POINTS_START[] = {
        _EX_BIN_OPERATOR,
        _EX_ASN_OPERATOR,
        _EX_CLOSE_BRACE,
        _EX_OPEN_BRACE,
        _EX_DELIM,  
        0
    };
    return contains_tok(tok, __BREAK_POINTS_START);    
}

/*
** restoration works by destroying a
** portion of the upper part of the stack.
**
** It will slice the top (newest) portion of the stack
** mark INCOMPLETE, and continue.
*/
struct RestorationFrame {
    /* points to storation point  */
    const struct Token * operator_stack_tok;
    const struct Token * output_tok;
    const struct Token * current;

    const struct Group * grp;
};

/*
** Grab the last inserted frame
*/
const struct RestorationFrame * restoration_head(const struct Parser *state) {
    return &((struct RestorationFrame *)state->restoration_stack.base)[(state->restoration_ctr || 1) - 1];
}

/*
** Returns boolean if restoration hook should be ran
**
*/
bool run_hook(enum Lexicon current){
    return is_delimiter(current)
        || is_open_brace(current)
        || is_close_brace(current);
}


void restoration_hook(struct Parser *state)
{
    struct RestorationFrame rframe;
    const struct Token *current = &state->src[*state->_i];

    if (run_hook(current->type))
    {
        /* pop last delimiter restore point  */
        if (is_delimiter(current->type))
            state->restoration_ctr -= 1;

        /* pop last delimiter & open brace */
        else if (is_close_brace(current->type))
            state->restoration_ctr -= 2;

        /* debug = Vec<struct Token*> */
        rframe.operator_stack_tok = state->operator_stack[state->operators_ctr];
        rframe.output_tok = vec_head(&state->debug);
        rframe.current = &state->src[*state->_i];

        vec_push(&state->restoration_stack, &rframe);
    }
}

void unwind_stacks(struct Parser *state)
{
    const struct Token *token;
    const struct RestorationFrame *rframe;
    struct Group *grp;
    uint16_t head = 0;

    /* get last good restoration */
    rframe = restoration_head(state);
    head = rframe->output_tok->seq;
    token = vec_head(&state->debug);

    // pop until out matches restoration
    while (state->debug.len > 0 && token->seq > head)
    {
        vec_pop(&state->debug, 0);
        token = vec_head(&state->debug);
    }

    head = rframe->operator_stack_tok->seq;
    token = state->operator_stack[state->operators_ctr];

    while(state->operators_ctr > 0 && token->seq > head)
    {
        state->operators_ctr -= 1;
        token = state->operator_stack[state->operators_ctr];
    }

    head = rframe->grp->seq;
    grp = &state->set_stack[state->set_ctr];

    while(state->set_ctr > 0 && grp->seq > head)
    {
        state->set_ctr -= 1;
        grp = &state->set_stack[state->set_ctr];
    }

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
    struct Parser *state,
    bool unexpected_token
){
    const struct Token *start;
    struct ParserError err;
    uint16_t continue_ctr = 0;

    // word + {1, 22 w}

    /* all other errors complete the main
    ** loop before panicing
    */
    start = &state->src[*state->_i - 1];
    if (unexpected_token)
      start = &state->src[*state->_i];

    /* if restoration point available */
    if (state->restoration_ctr > 0)
    {
        unwind_stacks(state);

        // pop until out matches restoration
        for (continue_ctr=*state->_i + 1; state->src_sz > continue_ctr; continue_ctr++)
        {
            if (is_continuable(state->src[continue_ctr].type))
                break;
        }

        if (continue_ctr == state->src_sz)
            return -1;

        else if (continue_ctr - *state->_i > 1)
        {
            err.span_t = ET_Span;
            err.inner.span.start = *start;
            err.inner.span.end = state->src[continue_ctr - 1];
            *state->_i = continue_ctr;
        } else {
            err.span_t = ET_Scalar;
            err.inner.scalar = state->src[(*state->_i) + 1];
            *state->_i += 1;
        }

    }

    else {
        state->set_ctr = 0;
        state->operators_ctr = 0;
        vec_clear(&state->debug);
    }

    state->panic_flags |= STATE_INCOMPLETE;
    vec_push(&state->errors, &err);
}
