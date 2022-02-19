#include <stdio.h>

#include "clonk.h"
#include "private.h"

enum Associativity
{
    NONASSOC,
    RASSOC,
    LASSOC
};

enum Associativity get_assoc(enum onk_lexicon_t token)
{
    switch(token) {
        case ONK_POW_TOKEN:
            return RASSOC;
        case ONK_NOT_TOKEN:
            return RASSOC;
        default:
            return LASSOC;
    }
}

/*
**********************************************
** When an operator is placed in the parser,
** it's order precedence is determined
** using shunting yard.
**
** Shunting yard uses an operator stack to
** determine evaluation order. If the precedence
** of the current operator (op1) is less than
** whats at the top/head of the stack (op2) then
** we'll drop operations out of the stack
** onto the output until the precedense of the op1 is
** equal (unless right-assciotated).
** or greater than op2.
**
** When operators are placed onto the output,
** they're created into expression. Each
** binary operator will pop 2 arguments
** from the top of the expression stack,
** and then the operator expression will
** be pushed onto the expression stack.
**
** postfix: ... <expr> <expr> <OPERATOR> ...
********************************************
*/
int8_t handle_operator(struct Parser *state)
{
  int8_t precedense = 0, head_precedense = 0;
  const struct onk_token_t *current = current_token(state),
    *head = 0;
  
  precedense = op_precedence(current->type);

  /*
    If no operators are in operators-stack,
    place it directly.

    If the head of the operator stack is an open brace
    we don't need to do anymore checks
    before placing the operator  
  */  
  if (state->operators_ctr == 0 || precedense == 0)
  {
    state->operator_stack[state->operators_ctr] = current;
    state->operators_ctr += 1;
    return 0;
  }

  /* Grab the head of the operators-stack */
  head = op_head(state);

  /* skip IN operator if apart of ForLoopBody */
  if(head->type == ForBody && current->type == IN)
    return 0;

  head_precedense = op_precedence(head->type);

  assert(precedense != -1 || head_precedense != -1);

  /*
    while `head` has higher precedence
    than our current token pop operators from
    the operator-stack into the output
  */
  while (head_precedense > 0
         && head_precedense >= precedense
         && state->operators_ctr > 0)
  {
    if (head_precedense == 0)
      break;
    
    /* pop operators off the stack into the output */
    if (head_precedense > precedense)
      insert(state, head);
    
    /*
        If left associated, push equal
        precedence operators onto the output
    */
    else if (precedense == head_precedense && get_assoc(current->type) == LASSOC) 
      insert(state, head);
    
    /* discard operator after placed in output */
    state->operators_ctr -= 1;

    if (state->operators_ctr == 0)
      break;

    head = state->operator_stack[state->operators_ctr - 1];
    head_precedense = op_precedence(head->type);
  }

  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;
  return 0;
}

/*
 * Groups take N amount of arguments from the stack where
 * N is derived from `struct Group`'s delmiter_ctr + 1.
 *
 * Grouping represents sets of data like
 * lists, tuples, & codeblocks.
 *
 * src: [body_expr, ...]
 * dbg: <body-expr> ... Group_t
*/
int8_t pop_group(struct Parser *state, bool do_checks)
{
  struct Group *ghead = group_head(state);

  if(do_checks && !onk_is_tok_open_brace(op_head(state)->type))
     return -1;

  /* only add groups if they're not singular item paramethesis braced */
  if (ghead->origin->type != ONK_PARAM_OPEN_TOKEN
      && delimiter_cnt != 1)
        push_output(
          state,
          ghead->type,
          ghead->expr_cnt
  );

  /* drop open brace */
  state->operators_ctr -= 1;
  /* drop group */
  state->set_ctr -= 1;

  return 0;
}

/*
** After a group is added to the output, there may be
**
*/
int8_t pop_block_operator(struct Parser *state)
{
  const struct onk_token_t *ophead;

  if(state->operators_ctr > state->operator_stack_sz
     || state->operators_ctr == 0)
     return -1;

  ophead = op_head(state);
  if (onk_is_tok_group_modifier(ophead->type))
  {
    insert(state, ophead);
    state->operators_ctr -= 1;
  }

  return 0;
}


int8_t handle_close_brace(struct Parser *state)
{
  const enum onk_lexicon_t expected[] = {ONK_COLON_TOKEN, ONK_DIGIT_TOKEN, 0};
  struct Group *ghead = group_head(state);
  const struct onk_token_t *prev = prev_token(state),
    *current = current_token(state);
  int8_t ret = 0;

  /* Operators stack is empty */
  assert(
    0 < state->operators_ctr && prev
    && state->set_sz > state->set_ctr
  );

  if (!onk_is_tok_delimiter(prev->type))
    ghead->expr_cnt += 1;

  if (prev->type == invert_brace_tok_ty(current->type))
    ghead->is_empty = true;

  /*
    flush out operators, until the open-brace
    type is found in the operator-stack.
  */
  if (flush_ops(state) == -1)
      return -1;

  if(ghead->type == onk_idx_group_token)
    finish_idx_access(state);

  /* handle grouping */
  ret = pop_group(state, false);

  /*TODO: move to predict.c*/
  /* on index access group cannot be empty */
  if (ghead->is_empty && ghead->type == onk_idx_group_token) {
      throw_unexpected_token(state, current, expected, 2);
      return -1;
  }

  /*
    After closing a code-block,
    there may be an operator declared on it,
    we'll check for it now.
  */
  if (state->operators_ctr > 0
      && pop_block_operator(state) == -1)
    return -1;

  return ret;
}



/*
** Handles group notation operations such as
** function call (`foo(x)`) & index access (`foo[x]`)
** `Init {a=b}`
** Places `Apply` before the open-brace type `(`
** when the opposite brace type is found,
** and the group is put into the output,
** `pop_block_operators` will be ran,
** and insert the token before it (`Apply`/`IndexAccess`)
** if that token is defined as a block-descriptor.
**
**  accepts the following patterns as function calls
**  where the current token is `(`
**       )(
**       ](
**    word(
**        ^-- current token.
**  accepts the following patterns as an array index
**    where the current token is `[`
**        )[
**        ][
**     word[
**        "[
**         ^-- current token.
**
**     word{
**         ^---current token.
**
*/
enum onk_lexicon_t push_group_modifier(
  const struct onk_token_t * start,
  const struct onk_token_t *prev
){
  const struct onk_token_t * current = current_token(state);
  const struct onk_token_t * prev = prev_token(state);

  if(!prev)
    return 0;

  switch (current->type)
  {
    case ONK_PARAM_OPEN_TOKEN:
      if(is_fncall_pattern(prev->type))
        return Apply;
      break;

    case ONK_BRACKET_OPEN_TOKEN:
      if(is_index_pattern(prev->type))
        return _IdxAccess;
      break;

    case ONK_BRACE_OPEN_TOKEN:
      if(prev->type == ONK_WORD_TOKEN)
        return StructInit;
      break;

    default:
      return 0;
  }

  return 0;
}

/*
  every opening brace starts a new group
*/
int8_t handle_open_brace(struct Parser *state)
{
  const struct onk_token_t *current = current_token(state);
  const struct onk_token_t *prev = prev_token(state);
  enum onk_lexicon_t modifier = push_group_modifier(current, prev);

  if (prev && modifier)
    push_op(modifier, 0, 0, state);

  /* look behind & insert group modifier if needed. */
  if (new_grp(state, current) == 0)
      return -1;
  
  // Place opening brace on operator stack
  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  return 0;
}

int8_t is_dual_grp_keyword(enum onk_lexicon_t tok) {
  switch(tok){
    case FOR: return 0;
    case WHILE: return 1;
    case IF: return 2;
    case FUNC_DEF: return 3;
    default: return -1;
  }
}
/*
   allows you to type in

   for x, y, z instead
*/
int8_t pretty_handle_for(
  struct onk_token_t *current,
  struct onk_token_t *next
){
  struct Group *ghead;
  const enum onk_lexicon_t expected;

  if(current->type == FOR)
  {
    // TODO: move to predict.c
    // Short
    if(next->type == onk_is_tok_open_brace(current->token)
       && next->token != ONK_PARAM_OPEN_TOKEN)
    {
      throw_unexpected_token(current, state, ONK_OPEN_PARAM_TOKEN, 1);
      return
    }
    else
    {
      push_op(OPEN_BRACE, 0, 0, state);
      ghead = new_grp(state);
      state->set_ctr += 1;

      if (current->token == FOR)
        ghead->stop_short[0] = IN;

      ghead->is_short = true;
      ghead->type = onk_tuple_group_token;

    }
  }

}


int8_t handle_dual_group(struct Parser *state)
{
  const struct onk_token_t *current = current_token(state);
  const struct onk_token_t *next = next_token(state);
  struct Group *ghead = group_head(state);
  const enum onk_lexicon_t products[4][3] = {
    {ForBody, ForParams, 0},
    {WhileBody, WhileCond, 0},
    {IfBody, IfCond, 0},
    {DefBody, DefSign, 0},
  };

  int8_t idx = is_dual_grp_keyword(current->type);
  assert(idx >= 0);

  ghead->expr_cnt += 1;
  push_many_ops(products[idx], current, state);

  if(current->type == FOR)
  {
    // TODO: move to predict.c
    // Short
    if(next->type == onk_is_tok_open_brace(current->token)
       && next->token != ONK_PARAM_OPEN_TOKEN)
    {
      throw_unexpected_token(current, state);
      return
    }
    else
    {
      push_op(OPEN_BRACE, 0, 0, state);
      ghead = new_grp(state);
      state->set_ctr += 1;

      if (current->token == FOR)
        ghead->stop_short[0] = IN;

      ghead->is_short = true;
    }
  }
}

/*
** consumes tokens until `import` token is found
** and converts it into a location token
**
** input: from ..word import new_word;
** output: ..word from new_word g(1) import
**
** input: import word;
** output word g(1) import
*/
int8_t handle_import(struct Parser *state)
{
  struct Group *gtop, *ghead = group_head(state);
  const struct onk_token_t *current = current_token(state);

  gtop = new_grp(state, next_token(state));
  gtop->is_short = true;

  if(output_head(state)->type != FROM)
    ghead->expr_cnt += 1;

  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  op_push(ONK_PARAM_OPEN_TOKEN, 0, 0, state);
  return 0;
}

/*
 * pop short-block
 *
 * short blocks are isolated
 * & inferred groups
 *
*/
void pop_short_block(struct Parser *state) {
  const struct onk_token_t *next = next_token(state);
  const struct Group *ghead = group_head(state);
  const struct onk_token_t *ophead, *gmod;

  while(group_head->is_short)
  {
    ophead = op_head(state);
    gmod = group_modifier(state);

    state->operator_ctr -= 1;
    assert(onk_is_tok_open_brace(ophead));

    if (onk_is_tok_group_modifier(gmod->type))
    {
      insert(state, ophead);
      state->operators_ctr -= 1;
    }

    state->set_ctr -= 1;
    ghead = group_head(state);
  }
}

/*
** turns `onk_partial_brace_group_token` into either `onk_map_group_token`
** or `onk_code_group_token`, invalid delimiter results in -1
** called in `handle_delimiter`
*/
int8_t complete_partial_gtype(struct Parser *state){
  struct Group *ghead = group_head(state);
  const struct onk_token_t *current = current_token(state);

  if (ghead->type == onk_partial_brace_group_token)
  {
    switch(current->type)
    {
      case ONK_COLON_TOKEN:
        ghead->type = onk_map_group_token;
        break;

      case ONK_SEMICOLON_TOKEN:
        ghead->type = onk_code_group_token;
        break;

      case ONK_COMMA_TOKEN:
        ghead->type = onk_struct_group_token;
        break;

      default: return -1;
    }
  }
  return 0;
}


/*
* flush the operator stack
* pop short blocks
*/
int8_t handle_delimiter(struct Parser *state)
{
  const enum onk_lexicon_t delim[2] = {ONK_COLON_TOKEN, ONK_SEMICOLON_TOKEN};
  struct Group *ghead = group_head(state);
  const struct onk_token_t
    *current = current_token(state),
    *prev = prev_token(state);

  ghead->delimiter_cnt += 1;
  ghead->last_delim = current;

  flush_ops(state);

  // TODO: see how this plays with if/for/while
  if (!onk_is_tok_delimiter(prev->type))
    ghead->expr_cnt += 1;

  //TODO: move to predict.c
  if (complete_partial_gtype(state) == -1)
  {
    throw_unexpected_token(state, current, delim, 2);
    return -1;
  }

  /* fill in empty index arg if non-specified*/
  else if(ghead->type == onk_idx_group_token && prev) {
    if (prev->type == ONK_COLON_TOKEN || prev->type == ONK_BRACKET_OPEN_TOKEN)
      push_output(state, ONK_NULL_TOKEN, 0);
  }

  if(ghead->is_short)
    pop_short_block(state);

  return 0;
}

int8_t handle_return(struct Parser *state)
{
  const struct onk_token_t *next = next_token(state);
  struct Group *group;
  struct onk_token_t *brace;

  /* push return onto operator stack */
  state->operator_stack[state->operators_ctr] = current;
  state->operators_ctr += 1;

  /*setup a group for its values*/
  if (!onk_is_tok_open_brace(next->type))
  {
    brace = push_op(OPEN_BRACE, 0, 0, state);
    group = new_grp(state, brace);
    group->is_short = true;

    /* group->stop_short = {ONK_SEMICOLON_TOKEN, ONK_BRACKET_CLOSE_TOKEN}; */
  }
}

/* does this keyword use a single group */
bool is_kw_single_group(enum onk_lexicon_t tok){
   tok == FROM
   || tok == ELSE
   || tok == STRUCT
   || tok == IMPL;
}

/*
   This function implements a "fancy" shunting yard parser.
   returning a vec of pointers, of the source tokens in postfix notation.

   "Fancy" means to include extra features that will be described below.
   These include the ulities needed to flourish the shunting yard
   algorithm into a full parser.

   prerequisite know-how:
     shunting yard parses infix arithmitic notation,
     and converts it into postfix arithmitic nottation.
     Infix notation: "1 + 2 + 3"
     postfix notation: "1 2 + 3 +"
     postfix notation is much easier to compute using a stack.

   Fancy features:
    ## Grouping:
       Grouping occurs when a collection of units or expressions is
       surrounded in braces and delimated by group's type delimiter.
       Grouping creates its own tokens where the token type is the following

       ```
       onk_tuple_group_token,
       onk_struct_group_token,
       onk_list_group_token,
       onk_idx_group_token,
       onk_map_group_token,
       onk_code_group_token
       ```
       groups store the amount of members in `struct onk_token_t->end`
       Groups can be defined as empty by marking `end` as 0.

       Example input: [1, 2, 3]; ()
       Example output: 1 2 3 onk_list_group_token(end=3) onk_tuple_group_token(end=0)

       # Tuple group
       Example: (a, b)
       Example output: a b onk_tuple_group_token(2)
       delimiter: ','
       Tuples are immutable collections of data

       # onk_struct_group_token
       Example: Name {a = b}
       Output: Name (A b =) onk_struct_group_token(1) StructInit(operator)
       delimiter: ','
       Structures define a new type of data,
       composed of other types.

       # onk_list_group_token
       Example: [a, b]
       Output: a b onk_list_group_token(2)
       delimiter: ','

       dynamically sized array

       # onk_idx_group_token
       Example: foo[1:2:3]
       Output: foo (1 2 3 onk_idx_group_token(3)) IndexAccess(operator)
       delimiter: ':'

       Creates an index selection
       on the previous expression.
       The numbers wrapped in braces represent the following
       parameters.

       ACCESSED [START:END:SKIP]

       The value in these parameters may be skipped,
       and the value will be inferred from min/max
       value based on the parameter.
       START - min(0)
       END - max(N items)
       skip - min(0)

       skipped argument example: foo[::2]

       # onk_map_group_token
       Example: {'a': 2, 'foo': 'bar'}
       Output: 'a' 2 'foo' 'bar' onk_map_group_token(4),
       Delimiter: ':' & ','

       HashMap are collections of keys, and data
       where each key is unique

       # Code block
       Example: { foo(); bar(); }
       Output: foo() bar() onk_code_group_token(2)
       Delimiter: ';'
       A collection of proceedures

   ## Extended Operator:

      ## `ONK_DOT_TOKEN` operator

      `ONK_DOT_TOKEN` is used to access properities of its parent structure.
      Inside the parser, `ONK_DOT_TOKEN` is treated as if its an binary operation.

      Exmaple input: abc.foo.last;
      Example output: abc foo . last .

      ## `Apply` operator
      `Apply` is used to call function
*/
int8_t parse(
  struct ParserInput *input,
  truct ParserOutput *out
){
  struct Parser state;
  struct onk_token_t *current;
  uint16_t i = 0;
  bool unexpected_token;

  assert(init_parser(&state, input, &i) == 0);

  for (i = 0 ;; i++) {
    current = &state.src[i];

    assert(state.operators_ctr > state.operator_stack_sz);
    unexpected_token = is_token_unexpected(&state);

    /*
      unexpected_token is a boolean used to determine if we
      failed on the previous loop, or the current one.

      if either state.panic or unexpected_token are true
      we begin unwinding
    */
    if(state.panic || unexpected_token)
      handle_unwind(&state, unexpected_token);

    /*
     * Units are the the foundational symbols
     * that represent proceedures & data
     * ONK_WORD_TOKENS, INTS, ONK_STRING_LITERAL_TOKEN
    */
    else if(onk_is_tok_unit(current->type))
      insert(&state, current);

    /*
     * Operations are pushed onto a stack,
     * and are popped off when an operator of greater
     * precedense is pushed onto the stack, or whenever
     * the operator-stack is flushed.
    */
    else if (onk_is_tok_operator(current->type))
      handle_operator(&state);

    /*
      Places 2 operators on the stack, that are
      popped when an open brace is
      placed onto the stack
    */
    else if(is_dual_grp_keyword(current->type))
      handle_dual_group(&state);

    /* flush operators and
     * pop an operator (open brace) & grouping */
    else if (onk_is_tok_close_brace(current->type))
      handle_close_brace(&state);

    /* push group modifier & open brace
     * onto the operator stack.
     * push another grouping */
    else if (onk_is_tok_open_brace(current->type))
      handle_open_brace(&state);

    /* flush operators, check for short grouping & pop it */
    else if (onk_is_tok_delimiter(current->type))
      handle_delimiter(&state);


    else if(current->type == IMPORT)
      handle_import(&state);

    /* pop from into output */
    else if(current->type == ONK_FROM_LOCATION_TOKEN)
    {
      insert(&state, current);
      insert(&state, op_head(&state));
      state.operators_ctr -= 1;
    }

    else if(current->type == RETURN)
      handle_return(&state);

    else if(current->type == ONK_EOFT)
      break;

    else {
#ifdef DEBUG
      printf("debug: token (%s) fell through precedence\n",
             onk_ptoken(tokens[i].type));
#endif
    }


    /* if (do_short_block(op_head(state)->type) */
    /*     && !onk_is_tok_open_brace(next_token(state)->type)) */
    /* { */
    /*     new_grp(state, push_op(OPEN_BRACE, 0, 0, &state)) */
    /*     group_head(state)->is_short = true; */
    /*     stop_short = {ONK_SEMICOLON_TOKEN, CLOSE_BRACE}; */
    /* } */

    if(!state.panic)
      restoration_hook(&state);
  }

  /* dump the remaining operators onto the output */
  flush_ops(&state);
  assert(state.operators_ctr == 1);

  return 0;
}
