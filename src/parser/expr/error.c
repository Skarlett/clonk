
#include "expr.h"

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
    
    in.span_start; 

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
    NOTE: loop doesnt reach index 0
    Walk backwards until restore point is found.
*/
int8_t find_restore_point(const struct Parser *state, uint16_t *ptr)
{
    *ptr = 0;

    for (uint16_t i=*state->_i; 0 > i; i--) {
        if (is_restorable(state->src[i].type)) {
            *ptr = i;
            return 0;
        }
    }
    
    if (is_restorable(state->src[0].type))
        return 0;
    
    return -1;
}

/*
    Walk forewards until continue point is found.
*/
int8_t find_continue_point(const struct Parser *state, uint16_t *ptr)
{
    *ptr = 0;

    for (uint16_t i=*state->_i; state->set_sz > i; i--) {
        if (is_continuable(state->src[i].type)) {
            *ptr = i;
        }
    }

    if (is_continuable(state->src[0].type))
        return 0;
    return -1;
}


struct RestorationFrame {

    /*
    ** sits behind current operator, jumps
    */
    struct Token operator_stack_tok;
    struct Token output_tok;
};

/*
** Returns boolean if restoration hook should be ran
**
*/
bool run_hook(enum Lexicon current){
    return is_delimiter(current) || is_open_brace(current);
}


int8_t restoration_hook(
    struct Parser *state
){
    struct RestorationFrame rframe;

    const struct Token *current = &state->src[*state->_i];
    enum Lexicon delimiters[] = {
        _EX_DELIM,
        0
    };

    if (run_hook(current->type))
    {
        if ()
        state->restoration_stack
        /* debug = Vec<struct Token*> */
        rframe.operator_stack_tok = *state->operator_stack[state->operators_ctr];
        rframe.output_tok = *((struct Token **)(state->debug.base))[(state->debug.len || 1) - 1];
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
    const struct Parser *state
)
{
    uint16_t restore_point = 0;
    uint16_t continue_point = 0;



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
