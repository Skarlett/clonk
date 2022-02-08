#include "expr.h"
#include "utils.h"
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
    return eq_any_tok(tok, __BREAK_POINTS_START);    
}

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


//TODO: watch out for tokens with `0`
//      marked as their sequence
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

void throw_unexpected_token(
  struct Parser *state,
  const struct Token *start,
  const enum Lexicon expected[],
  uint16_t nexpected
){

    struct PartialError *err = &state->partial_err;
    enum Lexicon *expect_heap;

    expect_heap = calloc(nexpected, sizeof(enum Lexicon));
    memcpy(expect_heap, expected, sizeof(enum Lexicon) * nexpected);

    err->type = parse_err_unexpected_token;
    err->start = *start;
    err->expect = expect_heap;
    err->nexpected = nexpected;

    state->panic=true;
}



void mk_unexpected_token_span(
    struct UnexpectedTokError *err,
    struct Token start,
    struct Token end
){
     err->selection.type = Union;
     err->selection.token.union_t.start = start;
     err->selection.token.union_t.end = end;
}

void mk_unexpected_token(
    struct UnexpectedTokError *err,
    struct Token token
)
{
    err->selection.type = Scalar;
    err->selection.token.scalar_t = token;
}


/*
 * creates a window of tokens
 * which are interpreted as bad
 * from unwinding
*/
void token_error_window(){


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
    struct UnexpectedTokError unexpected_tok;
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

        // pop until output matches restoration
        for (continue_ctr=*state->_i + 1; state->src_sz > continue_ctr; continue_ctr++)
        {
            if (is_continuable(state->src[continue_ctr].type))
                break;
        }

        if (continue_ctr == state->src_sz)
            return -1;

        else if (continue_ctr - *state->_i > 1)
        {
            mk_unexpected_token_span(
                &unexpected_tok,
                *start,
                state->src[continue_ctr - 1]
            );

            err.type = parse_err_unexpected_token;
            err.error.unexpected_tok = unexpected_tok;

            *state->_i = continue_ctr;
        } else {
            mk_unexpected_token(
                &unexpected_tok,
                state->src[*state->_i + 1]
            );

            err.type = parse_err_unexpected_token;
            err.error.unexpected_tok = unexpected_tok;

            *state->_i += 1;
        }

    }

    else {
        state->set_ctr = 0;
        state->operators_ctr = 0;
        vec_clear(&state->debug);
    }

    //state->panic_flags |= STATE_INCOMPLETE;
    vec_push(&state->errors, &err);
}



int8_t handle_error(
    struct Parser *state,
    struct ParseError *err,

    /* signifies if error
     * happened on the previous loop
     * In the case that this is false,
     * Its an unexpected token
     * foreseen by previsioning stage
    */
    bool prev_loop
) {

    //handle_unwind()


}
