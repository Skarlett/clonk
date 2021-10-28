#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "../../prelude.h"
#include "../lexer/lexer.h"
#include "../lexer/helpers.h"
#include "../lexer/debug.h"

#include "expr.h"
#include "pool.h"

#define OPERATOR_BUF_SZ 64
#define END_PRECEDENCE 127

enum Operation operation_from_token(enum Lexicon t){
    switch (t) {
        case ADD: return Add;
        case SUB: return Sub;
        case MUL: return Multiply;
        case DIV: return Divide;
        case MOD: return Modolus;
        case POW: return Pow;
        case AND: return And;
        case OR: return Or;
        case ISEQL: return IsEq;
        case ISNEQL: return NotEq;
        case GTEQ: return GtEq;
        case LTEQ: return LtEq;
        case LT: return Lt;
        case GT: return Gt;
        case DOT: return Access;
        default: return UndefinedOp;
    }
}

/*
  derive the group type from the brace token
  will always return `SetGroup` for `MapGroup` 
  and `SetGroup`

  returns `MarkerTUndef` (0) on error
*/
enum MarkerT derive_group_marker_t(enum Lexicon token) {
  if (token == PARAM_OPEN || token == PARAM_CLOSE)
    return TupleGroup;
  else if (token == BRACKET_OPEN || token == BRACKET_CLOSE)
    return ListGroup;
  else if (token == BRACE_OPEN || token == BRACE_CLOSE)
    return SetGroup;
  else
   return MarkerTUndef;
}

enum Associativity {
    NONASSOC,
    RASSOC,
    LASSOC
};

enum Associativity get_assoc(enum Lexicon token) {
    switch(token) {
        case POW:
            return RASSOC;
        case NOT:
            return RASSOC;
        default:
            return LASSOC;
    }
}

/*
    precendense table:
      ") ] }"   : 127 non-assoc
      "."     : 126 L
      
    
      "^"       : 6 R (1 ^ 2 ^ 3) -> (1 ^ (2 ^ 3))
      "/ * %"   : 5 L  (4 / 2 * 2) -> ((4 / 2) * 2)
      "+ -"     : 4 L
      "!= == >= > <= < && ||": 3 L
      "!"       : 2 R
      "( [ {"   : 0 non-assoc
*/
int8_t op_precedence(enum Lexicon token) {
    if (is_close_brace(token))
        return END_PRECEDENCE;
    
    if (token == DOT)
        return 126;
    
    else if (token == POW)
        return 6;

    else if (token == MUL 
        || token == DIV
        || token == MOD)
        return 5;

    else if (token == ADD
        || token == SUB)
        return 4;
    
    else if (token == ISEQL
        || token == ISNEQL
        || token == GTEQ
        || token == LTEQ
        || token == GT
        || token == LT
        || token == AND
        || token == OR
        || token == NOT)
        return 2;
    
    else if (is_open_brace(token))
        return 0;
    
    return -1;
}

// struct Token * new_token(struct StageInfixState *state, enum Lexicon token, usize start, usize end) {
//     struct Token *ret;
    
//     state->pool[state->pool_i].type = token;
//     state->pool[state->pool_i].start = start;
//     state->pool[state->pool_i].end = end;
    
//     ret = &state->pool[state->pool_i];
//     state->pool_i += 1;

//     return ret;
// }

int8_t add_marker(
  struct StageInfixState *state,
  enum MarkerT type,
  usize argc
){
  struct Token *ret;

  state->pool[state->pool_i].type = MARKER;
  state->pool[state->pool_i].start = type;
  state->pool[state->pool_i].end = argc;

  ret = &state->pool[state->pool_i];
  state->pool_i += 1;

  if (*state->out_ctr > state->out_sz)
    return -1;

  state->out[*state->out_ctr] = ret;
  *state->out_ctr += 1;

  return 0;
}

#define STACK_SZ 128
#define _OP_SZ 128


/*  Flushes operators of higher precedence than `current`
 *  into the output until stack is empty,
 *  or runs into an open brace token.
 *  This function will not pop off the open brace token
 *  if one
 */

int8_t flush_ops(struct StageInfixState *state)
{
  struct Token *head;

  if (state->operators_ctr <= 0)
    return 0;
  
  head = state->operator_stack[state->operators_ctr - 1];

  /* pop operators off of the operator-stack into the output */
  while (state->operators_ctr > 0) {
    /* ends if tokens inverted brace is found*/
    if (head->type == invert_brace_tok_ty(state->src[*state->i].type)) {
      
      /* do not discard opening brace yet
      operators_ctr -= 1; */
      break;
    }
    /* otherwise pop into output */
    else {
      if (*state->out_ctr > state->out_sz)
        return -1;
      state->out[*state->out_ctr] = head;
      *state->out_ctr += 1;
      state->operators_ctr -= 1;
    }

    if (state->operators_ctr <= 0)
      break;

    /* Grab the head of the stack */
    head = state->operator_stack[state->operators_ctr - 1];
  }

  return 0;
}

/*
    check flag, and if present, unset it.
*/
FLAG_T check_flag(FLAG_T set, FLAG_T flag){
    return set & flag;
}

void set_flag(FLAG_T *set, FLAG_T flag){
    *set = *set | flag;
}

void unset_flag(FLAG_T *set, FLAG_T flag){
    *set = *set & ~flag;
}

enum Lexicon get_expected_delimiter(struct StageInfixState *state) {
  /* setup delimiter expectation */
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];

  if(ghead->expecting_ctx == 0
    ||check_flag(CTX_SET, ghead->expecting_ctx)
    ||check_flag(CTX_LIST, ghead->expecting_ctx)
    ||check_flag(CTX_TUPLE, ghead->expecting_ctx)
  ) return COMMA;
  
  else if (check_flag(CTX_IDX, ghead->expecting_ctx))
    return COLON;
  
  else if (check_flag(CTX_MAP, ghead->expecting_ctx)) {
    if (ghead->delimiter_cnt % 2 == 0)
      return COMMA;
    else
      return COLON;
    // 2:3, 3:4
  }

  else
    return 0;
}

int8_t is_token_unexpected( struct StageInfixState *state, FLAG_T flags) {
  struct Token * current = &state->src[*state->i];
  enum Lexicon delim;
  FLAG_T check_list = 0;

  if (current->type == DOT)
    set_flag(&check_list, EXPECTING_APPLY_OPERATOR);
  
  else if (is_bin_operator(current->type))
    set_flag(&check_list, EXPECTING_ARITHMETIC_OPERATOR);
  
  else if (is_delimiter(current->type)) {
    delim = get_expected_delimiter(state);
    
    if (delim != current->type)
      return -1;

    set_flag(&check_list, EXPECTING_DELIMITER);
  }
  else {  
    switch (current->type) {
      case WORD:
        set_flag(&check_list, EXPECTING_SYMBOL);
        break;

      case NULLTOKEN:
        set_flag(&check_list, EXPECTING_SYMBOL);
        break;

      case STRING_LITERAL:
        set_flag(&check_list, EXPECTING_STRING);
        break;

      case PARAM_OPEN:
        set_flag(&check_list, EXPECTING_OPEN_PARAM);
        break;

      case PARAM_CLOSE:
        set_flag(&check_list, EXPECTING_CLOSE_PARAM);
        break;

      case BRACKET_OPEN:
        set_flag(&check_list, EXPECTING_OPEN_BRACKET);
        break;

      case BRACKET_CLOSE:
        set_flag(&check_list, EXPECTING_CLOSE_BRACKET);
        break;

      default: return -1;
    }
  }

  return !check_flag(flags, check_list);
}

FLAG_T expect_any_close_brace(){
  return 0 | EXPECTING_CLOSE_BRACKET
    | EXPECTING_CLOSE_PARAM
    | EXPECTING_CLOSE_BRACE;
}

FLAG_T expect_any_open_brace(){
  return 0 | EXPECTING_OPEN_BRACKET
    | EXPECTING_OPEN_PARAM
    | EXPECTING_OPEN_BRACE;
}

FLAG_T expect_any_data(){
  return 0 | EXPECTING_SYMBOL
    | EXPECTING_INTEGER
    | EXPECTING_STRING;
}

FLAG_T expect_any_op(){
  return 0 | EXPECTING_ARITHMETIC_OPERATOR
    | EXPECTING_APPLY_OPERATOR;
}

FLAG_T setup_flags(struct StageInfixState* state)
{
  struct Token *current = &state->src[*state->i];
  FLAG_T ret = FLAG_ERROR;

  if (is_symbolic_data(current->type))
  {
    set_flag(&ret, 0
      | EXPECTING_ARITHMETIC_OPERATOR      
      | expect_any_close_brace()
      | EXPECTING_DELIMITER);

    if (current->type == WORD)
      set_flag(&ret, EXPECTING_OPEN_BRACKET
        | EXPECTING_OPEN_PARAM
        | EXPECTING_APPLY_OPERATOR);
    
    else if(current->type == STRING_LITERAL)
      set_flag(&ret, EXPECTING_OPEN_BRACKET 
        | EXPECTING_APPLY_OPERATOR
      );
  }
  
  else if (current->type == DOT)
    //(a.b).
    set_flag(&ret, 0 
      | EXPECTING_SYMBOL
      | EXPECTING_NEXT);

  else if (is_bin_operator(current->type)) {
    set_flag(&ret, 0
      | expect_any_data()
      | expect_any_open_brace()
      | EXPECTING_NEXT
    );
    
    if (current->type != DOT)
      set_flag(&ret, EXPECTING_INTEGER
        | EXPECTING_STRING);
  }

  else if (current->type == COMMA || current->type == COLON)
    set_flag(&ret, 0
      | expect_any_data()
      | expect_any_open_brace()
      | EXPECTING_NEXT
    );
  
  else if (is_open_brace(current->type)) {
      set_flag(&ret, 0
        | expect_any_open_brace()
        | expect_any_data()
        | EXPECTING_NEXT
      );

      // TODO: invert current->type and expect it  
    }

  // ][[
  else if (is_close_brace(current->type))
     set_flag(&ret, 0
        | expect_any_close_brace()
        | EXPECTING_OPEN_BRACKET
        | EXPECTING_OPEN_PARAM
        | EXPECTING_DELIMITER
        | expect_any_op()
    );
  
  return ret;
}

int8_t handle_unwind(struct StageInfixState *state) {

    
}

/*
  symbols are placed directly into the output.
*/
int8_t handle_symbol(struct StageInfixState* state) {
    state->out[*state->out_ctr] = &state->src[*state->i];
    *state->out_ctr += 1;
    
    state->set_stack[state->set_ctr].atomic_symbols += 1;
    return 0;
}

/*
  every opening brace starts a new possible group,
  increment the group stack, and watch for the following 
  in-fix patterns.
   
  accepts the following patterns as function calls
  where the current token is `(`
       )(
       ](
    word(
        ^-- current token.

  accepts the following patterns as an array index
    where the current token is `[`
        )[
        ][
     word[
        "[
         ^-- current token.
*/
int8_t handle_open_brace(struct StageInfixState *state) {
  struct Group *ghead = 0;
  struct Token *current, *prev = 0, *next = 0;

  current = &state->src[*state->i];

  if (*state->i > 0)
    prev = &state->src[*state->i - 1];
  
  if (state->src_sz - 1 > *state->i)
    next = &state->src[*state->i + 1];

  /* overflow check */
  if (state->set_ctr > state->set_sz ||
      state->operators_ctr > state->operator_stack_sz)
  {
    set_flag(&state->state, INTERNAL_ERROR);
    return -1;
  }
  
  ghead = &state->set_stack[state->set_ctr];

  ghead->atomic_symbols = 0;
  ghead->delimiter = 0;
  ghead->delimiter_cnt = 0;
  ghead->origin = current;

  ghead->tag_op = 0;

  /*
    function call pattern
  */
  if (current->type == PARAM_OPEN)
  {
    ghead->expecting_ctx = 0 | CTX_TUPLE;

    if (prev && (is_close_brace(prev->type) ||
        prev->type == WORD)) 
      {
        ghead->tag_op = GOP_APPLY;
        ghead->index_accsess_operand_ctr += 1;
      }
  }

  /*
    index call pattern
  */
  else if (current->type == BRACKET_OPEN)
  {
    ghead->expecting_ctx = 0 | CTX_LIST;
    /* peek-behind to check for index access */
    if (prev && (is_close_brace(prev->type) ||
        prev->type == WORD ||
        prev->type == STRING_LITERAL))
    {
      ghead->tag_op = GOP_INDEX_ACCESS;
      ghead->index_accsess_operand_ctr += 1;
    }

  }
  
  else if (current->type == BRACE_OPEN)
  {
    ghead->expecting_ctx = 0 | CTX_SET;
  }

  /* increment group */
  state->set_ctr += 1;

  
  return 0;
}

int8_t handle_close_brace(struct StageInfixState *state) {
  struct Token *head, *prev;
  struct Group *ghead;
  enum MarkerT marker_type;

  // if (*state->i > 0){}
  
  // state->src[*state->i -1]
  /* Operators stack is empty */
  if (state->operators_ctr == 0
      /* unbalanced brace, extra closing brace.*/
      | state->set_ctr == 0)
    return -1;

  /* Grab the head of the group stack */
  ghead = &state->set_stack[state->set_ctr - 1];

  /* Grab the head of the operator stack */
  head = state->operator_stack[state->operators_ctr - 1];
  /*
      The current token is the closing brace
      of the top of operator-stack.
  */
  if (head->type == invert_brace_tok_ty(state->src[*state->i].type)) {
    if(ghead->atomic_symbols == 0)
    {
      /*
        its a set of 0 members/expressions
        Empty nestings may be symbolic for functionality,
        so include a GROUP(0) token in the output.
      */
      if (state->set_ctr > state->set_sz 
          || ghead->tag_op == GOP_INDEX_ACCESS)
        return -1;
      
      marker_type = derive_group_marker_t(state->src[*state->i].type);

      if (marker_type == 0 || 
          add_marker(state, marker_type, 0) == -1)
            return -1;
    }
  } 
  else
  {
    /* should be atleast one operator in the stack */
    if (state->operators_ctr == 0)
      return -1;

    flush_ops(state);
  }

  if (ghead->tag_op == INDEX_ACCESS) {
    /* Expects delimiter & brace type */
    if (ghead->delimiter != COLON || head != ghead->origin || 
        /* overflow check */
        *state->out_ctr + (5 - ghead->index_accsess_operand_ctr) > state->out_sz)
      return -1;
    
    // Fill in any missing arguments
    for (uint8_t i=ghead->index_accsess_operand_ctr; 4 > ghead->index_accsess_operand_ctr; i++) {
      state->out[*state->out_ctr] = new_token(state, NULLTOKEN, 0, 0);
      *state->out_ctr += 1;
    }

    state->out[*state->out_ctr] = new_token(
        state,
        INDEX_ACCESS,
        0,
        0
    );

    *state->out_ctr += 1;
  }

  else if (ghead->tag_op == APPLY) {
    if (head->type != PARAM_OPEN || ghead->origin != head
    || *state->out_ctr > state->out_sz)
      return -1;
    
    state->out[*state->out_ctr] = new_token(
        state,
        INDEX_ACCESS,
        0,
        ghead->delimiter_cnt + 1
    );

    *state->out_ctr += 1;
  }

  else {
    // TODO add GROUPING operand
  }

  /* discard opening brace from operator stack */
  state->operators_ctr -= 1;
  state->set_ctr -= 1;
  return 0;
}

int8_t handle_operator(struct StageInfixState *state) {
  struct Token *head;
  int8_t precedense = 0, head_precedense = 0;

  precedense = op_precedence(state->src[*state->i].type);

  /* unrecongized token */
  if (precedense == -1)
    return -1;

  /*
      no operators in operators-stack,
      so no extra checks needed

      if the head of the operator stack is an open brace
      we don't need to do anymore checks
      before placing the operator  
  */
  else if (state->operators_ctr == 0 || is_open_brace(head->type)) {
    state->operator_stack[state->operators_ctr] = &state->src[*state->i];
    state->operators_ctr += 1;
    return 0;
  }

  /* Grab the head of the operators-stack */
  head = state->operator_stack[state->operators_ctr - 1];

  /*
      while `head` has higher precedence
      than our current token pop operators from
      the operator-stack into the output
  */
  
  while (op_precedence(head->type) >= precedense && state->operators_ctr > 0) {
    head_precedense = op_precedence(head->type);

    if (is_open_brace(head->type))
      break;
    
    /*
        pop operators off the stack
        into the output
    */
    if (head_precedense > precedense) {
      if (*state->out_ctr > state->out_sz) 
        return -1;
      
      state->out[*state->out_ctr] = head;
      *state->out_ctr += 1;
    }

    /*
        If left associated, push equal
        precedence operators onto the output
    */
    else if (precedense == head_precedense) {
      if (get_assoc(state->src[*state->i].type) == LASSOC) {
        
        /* overflow check */
        if (*state->out_ctr > state->out_sz)
          return -1;
        
        state->out[*state->out_ctr] = head;
        *state->out_ctr += 1;
      }
    }

    /* discard operator after placed in output */
    state->operators_ctr -= 1;

    if (state->operators_ctr <= 0)
      break;

    head = state->operator_stack[state->operators_ctr - 1];
  }

  /* place low precedence operator */
  state->operator_stack[state->operators_ctr] = &state->src[*state->i];
  state->operators_ctr += 1;
  return 0;
}

int8_t handle_delimiter(struct StageInfixState *state) {
  struct Token *head = 0, 
    *current = &state->src[*state->i],
    *prev = 0,
    *next = 0;

  struct Group *ghead = 0;

  /* Setup group group head ptr */
  if (state->set_ctr > 0)
    ghead = &state->set_stack[state->set_ctr - 1];
  else
    ghead = &state->set_stack[0];

  /* increment the delimiter counter */
  ghead->delimiter_cnt += 1;
  
  if (*state->i > 0)
    prev = &state->src[*state->i - 1];
  
  if (state->src_sz > *state->i + 1)
    next = &state->src[*state->i + 1];
  
  if(!prev || !head)
    return -1;
  
  if (current->type == COLON) {
    /* if not expecting brace char?*/

    if (// overflow check
        state->set_ctr > state->set_sz ||
        // there must atleast one group ('[')
        1 > state->set_ctr ||
        // too many colons?
        ghead->delimiter_cnt > 2 ||
        // previously set to comma
        ghead->delimiter == COMMA ||
        // not allowed to index the previous thing
        ghead->tag_op != INDEX_ACCESS)
      return -1;
    
    set_flag(&ghead->delimiter, DELIM_COLON);

    /*
        peek behind our current token to 
        see if its a COLON, or an OPEN_BRACKET.

        If so, place a null token to transform `[:...]`
        into `[NULLTOKEN:...]`
    */
    if (prev->type == BRACKET_OPEN ||
        prev->type == COLON)
    {
      /*
        add null operand to output to pad idx access
        this also keeps us aligned to `ghead->idx_value_ctr`
      */
      state->out[*state->out_ctr] = new_token(state, NULLTOKEN, 0, 0);
      *state->out_ctr += 1; 
    }

    /*
      if last colon token, 
      peek ahead to see if theres a symbol
    */
    if (ghead->delimiter_cnt == 2 && next->type == BRACKET_CLOSE)
    {
      state->out[*state->out_ctr] = new_token(state, NULLTOKEN, 0, 0);
      *state->out_ctr += 1;
      ghead->index_accsess_operand_ctr += 1;
    }

    ghead->index_accsess_operand_ctr += 1;
  }

  else if (current->type == COMMA)
  {
    ghead->delimiter = COMMA;
  }
  
  /* set seen flag */
  if (current->type == COLON)
    set_flag(&ghead->delimiter, DELIM_COLON);
  
  else if(current->type == COMMA)
    set_flag(&ghead->delimiter, DELIM_COMMA);
  

  /*
    empty operators until ( [
  */
  flush_ops(state);
  return 0;
}

/*
  Shunting yard expression parsing algorthim 
  https://en.wikipedia.org/wiki/Shunting-yard_algorithm
  --------------

  This function takes takes a stream of token `tokens[]`
  and writes an array of pointers (of type `struct Token`)
  into `*output[]` in postfix notation.

  The contents of `*output[]` will be a
  POSTFIX notation referenced from the 
  INFIX notation of `input[]`.

    infix: 1 + 1
  postfix: 1 1 +
    input: [INT, ADD, INT]
   output: [*INT, *INT, *ADD]

  Further more, this function handles organizing operation precedense
  based on shunting-yard algorthm.
  This is in combination with arithmetic operations, and our custom operations
  (GROUP, INDEX_ACCESS, APPLY, DOT).
  
  Upon completion, the result will be an ordered array of operands, 
  and operators ready to be evaluated into a tree structure.

  infix:   (1+2) * (1 + 1)
  postfix: 1 2 + 1 1 + *
  To turn the output into a tree see `stage_postfix_parser`.

  Digging deeper into the realm of this, 
  you'll find I evaluate some custom operators
  such as the DOT token, and provide 
  extra operators to the output to describe
  function calls (APPLY(N)).

  infix:   foo(a, b+c).bar(1)
  postfix  foo a b c + APPLY(3) bar 1 APPLY(2) .
  pretty-postfix:
           ((foo a (b c +) APPLY(3)) bar 1 APPLY(2) .)
 
  See src/parser/lexer/lexer.h#Lexicon::APPLY for 
  more information about the APPLY operator, and
  others like it.
*/
int8_t stage_infix_parser(
    struct Token tokens[], usize expr_size,
    struct Token *output[], usize output_sz,
    usize *output_ctr,
    struct Token token_pool[], usize pool_sz,
    struct CompileTimeError *err)
{
  struct StageInfixState state;
  struct Token *operators[STACK_SZ];
  struct Group groups[STACK_SZ];
  struct Token *head;

  int8_t ret = -1;
  int8_t precedense = 0;
  usize i = 0;

  state.i = &i;
  state.src = tokens;
  state.src_sz = expr_size;

  state.out = output;
  state.out_ctr = output_ctr;
  state.out_sz = output_sz;

  state.pool = token_pool;
  state.pool_sz = pool_sz;

  state.set_stack = groups;
  state.set_ctr = 0;
  state.set_sz = STACK_SZ;

  state.operators_ctr = 0;
  state.operator_stack = operators;
  state.operator_stack_sz = STACK_SZ;
  state.state = 0;

  set_flag(&state.expecting,  EXPECTING_OPEN_PARAM 
    | EXPECTING_STRING | EXPECTING_SYMBOL
    | EXPECTING_OPEN_BRACKET 
    | EXPECTING_OPEN_BRACE
    | EXPECTING_NEXT
    | EXPECTING_INTEGER
  );

  for (i = 0; expr_size > i; i++) {
    if (*output_ctr > output_sz ||
        state.operators_ctr > state.operator_stack_sz)
      return -1;

    else if (check_flag(state.state, STATE_PANIC) || is_token_unexpected(&tokens[i], state.expecting))
      handle_unwind(&state);

    else if (is_symbolic_data(state.src[i].type))
      handle_symbol(&state);

    else if (is_bin_operator(state.src[i].type))
      handle_operator(&state);

    else if (is_close_brace(state.src[i].type))
      handle_open_brace(&state);

    else if (is_open_brace(state.src[i].type))
      handle_open_brace(&state);

    else if (state.src[i].type == COLON || state.src[i].type == COMMA)
      handle_delimiter(&state);

    else {
#ifdef DEBUG
      printf("debug: token fell through precedense [%s]\n",
             ptoken(tokens[i]->type));
#endif
    }
  }
  
  /* setup next-token expectation */
  state.expecting = setup_flags(&state);
  
  /* dump the remaining operators onto the output */
  while (state.operators_ctr > 0) {
    head = operators[state.operators_ctr - 1];
    /*
        any remaining params/brackets/braces are unclosed
        indiciate invalid expressions
    */
    if (*state.out_ctr > state.out_sz || is_open_brace(head->type))
      return -1;

    state.out[*state.out_ctr] = head;

    *state.out_ctr += 1;
    state.operators_ctr -= 1;
  }

  return 0;
}

/* no special algorithms, just an equality test */
void determine_return_ty(struct Expr *bin) {
    if (bin->inner.bin.rhs->datatype == bin->inner.bin.lhs->datatype)
        bin->inner.bin.returns = bin->inner.bin.rhs->datatype;
    else
        bin->inner.bin.returns = UndefT;
}

struct StagePostfixState {
  const char *line;
  struct Token **postfix;
  struct Token *src;
  usize src_sz;
  usize *i;

  struct ExprPool *pool;

  struct Expr **stack;
  uint16_t stack_ctr;
  uint16_t stack_sz;
};

int8_t inc_stack(struct StagePostfixState *state, struct Expr ex)
{
  struct Expr *phandle;
  
  if (state->stack_ctr > state->stack_sz)
    return -1;

  phandle = pool_push(state->pool, ex);
  if (phandle == 0)
    return -1;
  
  state->stack[state->stack_ctr] = phandle;
  state->stack_ctr += 1;

  return 0;
}

int8_t mk_sym(struct StagePostfixState *state, struct Expr ex)
{
  struct Token *current = state->postfix[*state->i];
  uint8_t size = current->end - current->start;

  ex.type = SymExprT;
  ex.datatype = UndefT;
  ex.inner.symbol = calloc(1, size+1);
  
  /*bad alloc*/
  if (ex.inner.symbol == 0)
    return -1;

  memcpy(ex.inner.symbol,
    state->line + current->start,
    size + 1
  );
      
  ex.inner.symbol[size] = 0;

  if (inc_stack(state, ex) == -1)
    return -1;
  
  return 0;
}

int8_t mk_null(struct StagePostfixState *state, struct Expr ex) {
  ex.type = LiteralExprT;
  ex.datatype = NullT;
  
  if (inc_stack(state, ex) == -1)
    return -1;
  
  return 0;
}

int8_t mk_binop(struct StagePostfixState *state, struct Expr ex) {
  struct Expr *phandle;

  if (1 > state->stack_ctr)
    return -1;

  ex.type = BinaryExprT;
  ex.inner.bin.lhs = state->stack[state->stack_ctr - 1];
  ex.inner.bin.rhs = state->stack[state->stack_ctr - 2];
  ex.inner.bin.op = operation_from_token(state->postfix[*state->i]->type);
  determine_return_ty(&ex);
  state->stack_ctr -= 2;
  
  if (inc_stack(state, ex) == -1)
    return -1;
  
  return 0;
}

int8_t mk_int(struct StagePostfixState *state, struct Expr ex) {
  struct Expr *phandle;
  char * end;
  ex.type = LiteralExprT;
  ex.datatype = IntT;
  errno = 0;

  ex.inner.value.literal.integer =
    // TODO
    // shouldnt this be (usize specified) long long ???
    strtol(state->line + state->postfix[*state->i]->start, &end, 10);

  if (errno != 0 || end != state->line + state->postfix[*state->i]->end)
    return -1;

  if (inc_stack(state, ex) == -1)
    return -1;
  
  return 0;
}

int8_t mk_str(struct StagePostfixState *state, struct Expr ex) {
  struct Expr *phandle;
  struct Token *current = state->postfix[*state->i];
  usize size = current->end - current->start;

  ex.type = LiteralExprT;
  ex.datatype = StringT;
  
  ex.inner.value.literal.string =
      calloc(1, size + 1);

  memcpy(ex.inner.value.literal.string, &state->line[current->start],
        size);

  ex.inner.value.literal.string[size] = 0;
  
  if(inc_stack(state, ex) == -1)
    return -1;
  
  return 0;
}

int8_t mk_group(struct StagePostfixState *state, struct Expr ex) {
  struct Token *current = state->postfix[*state->i];

  if (state->stack_ctr > STACK_SZ)
        return -1;

  ex.inner.value.literal.grouping.ptr =
    malloc(sizeof(struct Expr) * current->end);
      
  ex.inner.value.literal.grouping.capacity = current->end;
  ex.inner.value.literal.grouping.length = current->end;
  ex.inner.value.literal.grouping.brace =
    state->postfix[current->start]->type;

  if (ex.inner.value.literal.grouping.ptr == 0)
    return -1;

  for (usize j = 0; current->end > j; j++) {
    ex.inner.value.literal.grouping.ptr[j] = state->stack[state->stack_ctr - 1];
    state->stack_ctr -= 1;
  }
    
  if (inc_stack(state, ex) == -1)
    return -1;

  return 0;
}

int8_t mk_fncall(struct StagePostfixState *state, struct Expr ex) {
  struct Token *current = state->postfix[*state->i];
  struct Expr *head;

  if (state->stack_ctr > STACK_SZ || current->end > MAX_ARGS_SZ)
    return -1;
    
  head = state->stack[state->stack_ctr - 1];

  ex.type = FnCallExprT;
  ex.datatype = 0;

  /* fill in arguments popping them off the stack */
  while (current->end > ex.inner.fncall.args_length) {
    ex.inner.fncall.args[ex.inner.fncall.args_length] = head;
    ex.inner.fncall.args_length += 1;
    state->stack_ctr -= 1;
    head = state->stack[state->stack_ctr - 1];
  }

  ex.inner.fncall.func_name = "anonymous";

  /* this is usually the function name, but could be a grouping */
  if (head->type == SymExprT)
    ex.inner.fncall.func_name = head->inner.symbol;
  else if (head->type == LiteralExprT &&
           head->datatype == GroupT)
    nop;
  else if (head->type == FnCallExprT)
    nop;
  else
    /*expected a group, or symbol as last item on the stack*/
    return -1;

  ex.inner.fncall.caller = head;
  state->stack_ctr -= 1;

  if (inc_stack(state, ex) == -1)
    return -1;
  return 0;
}

int8_t mk_idx_access(struct StagePostfixState *state, struct Expr ex) {
  ex.type = FnCallExprT;
  ex.datatype = 0;

  if (3 > state->stack_ctr)
    return -1;

  ex.inner.idx.start   = state->stack[state->stack_ctr - 1];
  ex.inner.idx.end     = state->stack[state->stack_ctr - 2];
  ex.inner.idx.skip    = state->stack[state->stack_ctr - 3];
  ex.inner.idx.operand = state->stack[state->stack_ctr - 4];

  state->stack_ctr -= 4;
  if (inc_stack(state, ex) == -1)
    return -1;
  return 0;
}

int8_t mk_not(struct StagePostfixState *state, struct Expr ex) {
  if (0 > state->stack_ctr)
    return -1;

  ex.inner.not_.operand = state->stack[state->stack_ctr - 1];
  state->stack_ctr -= 1;

  if (inc_stack(state, ex) == -1)
    return -1;
  return 0;
}

int8_t stage_postfix_parser(
  const char *line, 
  struct Token *tokens[],
  usize ntokens,
  struct ExprPool *pool
){
  struct Expr *stack[STACK_SZ];
  struct Expr ex;
  int8_t ret = 0;
  usize i = 0;

  struct StagePostfixState state;
  
  state.pool = pool;
  state.stack_ctr = 0;
  state.i = &i;
  state.postfix = tokens;
  state.line = line;

  if (!line || !tokens || !pool)
    return -1;

  for (i = 0; ntokens > i; i++)
  {
    memset(&ex, 0, sizeof(struct Expr));

    if (state.stack_ctr > state.stack_sz)
      return -1;

    else if (tokens[i]->type == WORD)
      ret = mk_sym(&state, ex);

    else if (tokens[i]->type == NULLTOKEN)
      ret = mk_null(&state, ex);

    else if (tokens[i]->type == INTEGER)
      ret = mk_int(&state, ex);

    else if (tokens[i]->type == STRING_LITERAL)
      ret = mk_str(&state, ex);

    else if (is_bin_operator(tokens[i]->type))
      ret = mk_binop(&state, ex);

    else if (tokens[i]->type == GROUPING)
      ret = mk_group(&state, ex);

    else if (tokens[i]->type == APPLY)
      ret = mk_fncall(&state, ex);

    else if (tokens[i]->type == INDEX_ACCESS)
      ret = mk_idx_access(&state, ex);
    
    else if (tokens[i]->type == NOT)
      ret = mk_not(&state, ex);
    
    else
      return -1;

    if (ret == -1)
      return -1;
  }
  return 0;
}

// int8_t new_expr(char *line, struct Token tokens[], usize ntokens, struct Expr *expr) {
//     /*todo: create expr tree*/
//     return 0;
// }
