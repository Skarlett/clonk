
#include "expr.h"
#include <string.h>
#include "../../error.h"
#include <assert.h>
#include <stdint.h>

int8_t mk_parser_error(
    struct Parser *state,
    enum Severity severity,
    const char * msg
){
    struct ParseError in;
    struct Error err;
    
    // in.span_start = 
    // in.span_end =

    mk_error(&err, &in, ErrorParserT, severity, msg);
    vec_push(&state->errors, &err);
    return 0;
}

int8_t mk_unexpected_error(
    struct Parser *state,
    const char * msg
) {
    struct ParseError in;
    struct Error err;
    
//    in.span_start;

    mk_error(&err, &in, ErrorParserT, Error, msg);
    vec_push(&state->errors, &err);


}

/**
 * @param tok: comparsion token
 * @param start_or_end: 0 (start) or 1 (end)
 */
int8_t is_restorable(enum Lexicon tok) {
    static enum Lexicon __BREAK_POINTS[] = {
        _EX_CLOSE_BRACE,
        _EX_OPEN_BRACE,
        _EX_DELIM,
        0
    };

    return contains_tok(tok, __BREAK_POINTS);
}

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
    Walk forewards until continue point is found.
*/
int8_t find_continue_point(const struct Parser *state, uint16_t *ptr)
{
    *ptr = 0;

    for (uint16_t i=*state->_i; state->set_sz > i; i++) {
        if (is_continuable(state->src[i].type)) {
            *ptr = i;
        }
    }

    if (is_continuable(state->src[0].type))
        return 0;
    return -1;
}


/*
**
**
*/
struct RestorationFrame {
    /*
    ** sits behind current operator, jumps
    */
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
    const struct Token *current = &state->src[*state->_i];
    struct RestorationFrame rframe;

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

/*
  unwind the parser to a previous safe-state
  ----
  1) find break point.
  2) undo stack instrutions
  3) find break point past intrusion
  4) report error
  5) continue parsing

*/
int8_t handle_unwind(
    struct Parser *state,
    struct ParserError *err,
    bool unexpected_char
){
    struct Token buf;
    struct TokenSpan buf_span;

    const struct RestorationFrame *rframe;
    const struct Token *token;
    struct Group *grp;
    uint16_t i = 0;
    uint16_t head = 0;
    // err->type = parse_err_unexpected_token;

    if (state->restoration_ctr > 0)
    {
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


        for (uint16_t i=*state->_i + 1; state->src_sz; i++)
        {
            if (is_continuable(state->src[i].type))
                break;
        }

        if (i - *state->_i > 1)
        {
            err->span_t = Span;
            err->inner.span.start = ;
        }
        else {
            err->inner.scalar = state->src[(*state->_i) + 1];
        }




    }
    //else {
//
//    }

    if (restore_point == 0)


      //todo
      //destory_state();

    /* [OPEN_PARAM, ADD, SUB, ...]*/
    state->operator_stack;


    /* [BinOpExpr,  ...]*/
    //state->expr_stack;

}

/*
** Keeps track of restoration points
**
*/
int8_t unwind_process_hook() {

}
