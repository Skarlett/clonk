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
#include "../../utils/vec.h"
#include "expr.h"
#include "expect.h"

int8_t mk_error(struct ExprParserState *state, enum ErrorT type, const char * msg) {
  struct CompileTimeError *err;
  
  err->type = type;
  err->msg = msg;
  if (vec_push(&state->errors, &state->src[*state->_i]) == 0)
    return -1;
  
  set_flag(&state->panic_flags, STATE_PANIC | STATE_INCOMPLETE);
  return 0;
}

int8_t throw_internal_error(struct ExprParserState *state, const char * meta, const char * msg)
{
  char * internal_msg;
  //TODO

#ifdef DEBUG
  internal_msg = malloc(strlen(meta) + strlen(msg));
  strcat(internal_msg, meta);
  strcat(internal_msg, msg);
#else
  internal_msg = msg;
#endif
  set_flag(&state->panic_flags, STATE_PANIC | STATE_INCOMPLETE | INTERNAL_ERROR);
  if (mk_error(state, Fatal, internal_msg) == -1)
    return -1;
  return 0;
}

#define throw_internal_error(X, MSG) throw_internal_error(X, FILE_LINE, MSG)

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
        case EQUAL: return Assign;
        case PLUSEQ: return AssignAdd;
        case MINUSEQ: return AssignSub;
        default: return UndefinedOp;
    }
}

/*
  derive the group type from the brace token
  will always return `SetGroup` for `MapGroup` 
  and `SetGroup`

  returns `MarkerTUndef` (0) on error
*/
enum Lexicon get_dbg_group_token(enum Lexicon token) {
  if (token == PARAM_OPEN || token == PARAM_CLOSE)
    return TupleGroup;
  else if (token == BRACKET_OPEN || token == BRACKET_CLOSE)
    return ListGroup;
  else if (token == BRACE_OPEN || token == BRACE_CLOSE)
    return SetGroup;
  else
   return TOKEN_UNDEFINED;
}

int8_t add_dbg_sym(
  struct ExprParserState *state,
  enum Lexicon type,
  usize argc
){
  struct Token marker, *ret;

  marker.type = type;
  marker.start = 0;
  marker.end = argc;
  
  // push marker in the pool
  ret = vec_push(&state->pool, &marker);
  
  // put ptr of marker's new allocation in
  // the debug output
  if (ret == 0 || vec_push(&state->debug, &ret) == 0)  
  {
    throw_internal_error(state, "allocation failure in vec.");
    return -1;
  }
  return 0;
}

int8_t inc_stack(
  struct ExprParserState *state,
  struct Expr *ex,
  bool add_debug)
{
  struct Expr * heap_ex;
  
  if (state->expr_ctr > state->expr_sz)
  {
    throw_internal_error(state, "Expr/debug ctr overflowed.");
    return -1;
  }
  
  heap_ex = vec_push(&state->expr_pool, &ex);

  if (heap_ex == 0 || (add_debug && vec_push(&state->debug, &state->src[*state->_i]) == 0))
  {
    throw_internal_error(state, "vec pool returned null ptr.");
    return -1;
  }
  
  state->expr_stack[state->expr_ctr] = heap_ex;
  state->expr_ctr += 1;

  return 0;
}

int8_t handle_int(struct ExprParserState* state)
{
  struct Expr ex;
  struct Token *current = &state->src[*state->_i];
  usize size = current->end - current->start;

  char * end;
  ex.type = LiteralExprT;
  ex.datatype = IntT;
  errno = 0;

  ex.inner.value.literal.integer =
    str_to_isize(state->line + current->start, &end, 10);

  if (errno != 0) {
    throw_internal_error(state, "Didn't convert integer correctly");
    return -1;
  }

  // else if (end != state->line + size) {
  //   throw_internal_error(state, "Didn't parse integer correctly");
  //   return -1;
  // }

  if (inc_stack(state, &ex, true) == -1)
    return -1;
  
  return 0;
}

/*
  symbols are placed directly into the output.
*/
int8_t handle_symbol(struct ExprParserState* state) {
  struct Expr ex;
  struct Token *current = &state->src[*state->_i];
  uint8_t size = current->end - current->start;
  
  ex.type = SymExprT;
  ex.datatype = UndefT;
  ex.inner.symbol = calloc(1, size+1);
  
  /*bad alloc*/
  if (ex.inner.symbol == 0) {
    throw_internal_error(state,  "Allocation failure");
    return -1;
  }
    
  memcpy(ex.inner.symbol,
    state->line + current->start,
    size + 1
  );
  
  ex.inner.symbol[size] = 0;
  
  if (inc_stack(state, &ex, true) == -1)
    return -1;
  return 0;
}

/* no special algorithms, just an equality test */
void determine_return_ty(struct Expr *bin) {
  if (bin->inner.bin.rhs->datatype == bin->inner.bin.lhs->datatype)
    bin->inner.bin.returns = bin->inner.bin.rhs->datatype;
  else
    bin->inner.bin.returns = UndefT;
}

int8_t mk_binop(struct ExprParserState *state, struct Expr *ex) {
  struct Token *current = &state->src[*state->_i];

  if (1 > state->expr_ctr)
  {
    throw_internal_error(state, "Not enough items on expr stack to build a binop");
    return -1;
  }

  ex->type = BinaryExprT;
  ex->inner.bin.lhs = 
    state->expr_stack[state->expr_ctr - 1];

  ex->inner.bin.rhs = 
    state->expr_stack[state->expr_ctr - 2];
  
  ex->inner.bin.op = operation_from_token(current->type);
  
  determine_return_ty(ex);

  state->expr_ctr -= 2;
  return 0;
}

int8_t mk_not(struct ExprParserState *state, struct Expr *ex) {

  if (0 > state->expr_ctr)
  {
    throw_internal_error(state, "Not enough items on expr stack to build a binop");
    return -1;
  }
  
  ex->type = NopExprT;
  ex->inner.not_.operand = state->expr_stack[state->expr_ctr - 1];
  state->expr_ctr -= 1;

  return 0;
}

int8_t mk_operator(
  struct ExprParserState *state,
  struct Expr *ex,
  struct Token *op_head
) {  
  if (op_head->type == NOT && mk_not(state, ex) == -1)
    return -1;
      
  else if (mk_binop(state, ex) == -1)
    return -1;
  
  return 0;
}

/*  Flushes operators of higher precedence than `current`
 *  into the output until stack is empty,
 *  or runs into an open brace token.
 *  This function will pop off the open brace token
 *  if one is found.
 */
int8_t flush_ops_until_br(struct ExprParserState *state)
{
  struct Token *head;
  struct Expr ex;

  if (state->operators_ctr <= 0)
    return 0;
  
  head = state->operator_stack[state->operators_ctr - 1];

  /* pop operators off of the operator-stack into the output */
  while (state->operators_ctr > 0) {
    memset(&ex, 0, sizeof(struct Expr));
    /* ends if tokens inverted brace is found*/
    if (head->type == invert_brace_tok_ty(state->src[*state->_i].type))
      break;
    
    /* otherwise pop into output */
    else {
      if (mk_operator(state, &ex, head) == -1 || inc_stack(state, &ex, true) == -1)
        return -1;
      
      state->operators_ctr -= 1;
    }

    if (state->operators_ctr <= 0)
      break;

    /* Grab the head of the stack */
    head = state->operator_stack[state->operators_ctr - 1];
  }
  
  /* discard opening brace yet */
  state->operators_ctr -= 1;
  return 0;
}

int8_t flush_all_ops(struct ExprParserState *state) {
  /* dump the remaining operators onto the output */
  struct Token *head;
  struct Expr ex;

  while (state->operators_ctr > 0)
  {
    head = state->operator_stack[state->operators_ctr - 1];
    /*
        any remaining params/brackets/braces are unclosed
        indiciate invalid expressions
    */
    if (is_open_brace(head->type))
      return -1;

    if (mk_operator(state, &ex, head) == -1
        || inc_stack(state, &ex, true) == -1)
        return -1;
    
    state->operators_ctr -= 1;
  }

  return 0;
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
      "!"       : 2 R
      "!= == >= > <= < && ||": 2 L
      "+= -= =" : 1 R
      "( [ {"   : 0 non-assoc
*/

#define END_PRECEDENCE 127
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
    
    else if (token == EQUAL
      || token == PLUSEQ
      || token == MINUSEQ)
      return 1;
    
    else if (is_open_brace(token))
        return 0;
    
    return -1;
}

int8_t handle_operator(struct ExprParserState *state) {
  struct Expr ex;
  struct Token *head=0, *current=0;
  int8_t precedense = 0, head_precedense = 0;

  current = &state->src[*state->_i];
  if (state->operators_ctr > 0)
    head = state->operator_stack[state->operators_ctr-1];
  
  precedense = op_precedence(current->type);

  if (precedense == -1){
    throw_internal_error(state, "Unrecongized operator");
    return -1;
  }

  /*
    no operators in operators-stack,
    so no extra checks needed

    if the head of the operator stack is an open brace
    we don't need to do anymore checks
    before placing the operator  
  */
  else if (state->operators_ctr == 0 || is_open_brace(head->type)) {
    if (state->operators_ctr > state->operator_stack_sz)
    {
      throw_internal_error(state, "Operator stack overflowed.");
      return -1;
    }    
    state->operator_stack[state->operators_ctr] = current;
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
    memset(&ex, 0, sizeof(struct Expr));

    head_precedense = op_precedence(head->type);

    if (is_open_brace(head->type))
      break;
    
    /* pop operators off the stack into the output */
    if (head_precedense > precedense)
    {
      if (mk_operator(state, &ex, head) == -1 
        || inc_stack(state, &ex, true) == -1)
        return -1;
    }

    /*
        If left associated, push equal
        precedence operators onto the output
    */
    else if (precedense == head_precedense && get_assoc(current->type) == LASSOC) 
    {
      if (mk_operator(state, &ex, head) == -1
          || inc_stack(state, &ex, true) == -1)
          return -1;
    }

    /* discard operator after placed in output */
    state->operators_ctr -= 1;

    if (state->operators_ctr <= 0)
      break;

    head = state->operator_stack[state->operators_ctr - 1];
  }

  /* place low precedence operator */
  if (state->operators_ctr > state->operator_stack_sz)
  {
    throw_internal_error(state, "Operator stack overflowed.");
    return -1;
  }    
  
  state->operator_stack[state->operators_ctr] = &state->src[*state->_i];
  state->operators_ctr += 1;
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
int8_t handle_open_brace(struct ExprParserState *state) {
  struct Group *ghead = 0;
  struct Token *current, *prev = 0, *next = 0;

  current = &state->src[*state->_i];

  if (*state->_i > 0)
    prev = &state->src[*state->_i - 1];
  
  if (state->src_sz - 1 > *state->_i)
    next = &state->src[*state->_i + 1];

  /* overflow check */
  if (state->set_ctr > state->set_sz ||
      state->operators_ctr > state->operator_stack_sz)
  {
    throw_internal_error(state, "Internal group/operator stack overflowed.");
    return -1;
  }
  
  ghead = state->set_stack[state->set_ctr];
  /* increment group */
  state->set_ctr += 1;

  ghead->delimiter_cnt = 0;
  ghead->origin = current;
  /*
    function call pattern
  */
  if (current->type == PARAM_OPEN)
  {
    ghead->state = 0 | GSTATE_CTX_TUPLE | GSTATE_CTX_LOCK;

    if (prev) {
      if (is_close_brace(prev->type) || prev->type == WORD)
        set_flag(&ghead->state, GSTATE_OP_APPLY);
      
      // Looking behind is easier than looking ahead
      // lessons learned.
      else if (prev->type == IF)
        set_flag(&ghead->state, GSTATE_OP_IF_COND);
    }
  }
  
  /*
    index call pattern
  */
  else if (current->type == BRACKET_OPEN)
  {
    ghead->state = 0 | GSTATE_CTX_LIST | GSTATE_CTX_LOCK;
    /* peek-behind to check for index access */
    if (prev && (is_close_brace(prev->type)
       ||prev->type == WORD
       ||prev->type == STRING_LITERAL))
          set_flag(&ghead->state, GSTATE_OP_IDX);
  }
  else if (current->type == BRACE_OPEN) { 
    ghead->state = 0 | GSTATE_CTX_SET;

  }
  if (!check_flag(ghead->state, GSTATE_OP_APPLY)
      ||!check_flag(ghead->state, GSTATE_OP_IDX))
        set_flag(&ghead->state, GSTATE_OP_GROUP);
  
  return 0;
}

enum GroupT get_group_ty(FLAG_T group_state) {
  if (check_flag(group_state, GSTATE_CTX_TUPLE))
    return TupleT;
  else if(check_flag(group_state, GSTATE_CTX_SET))
    return SetT;
  else if (check_flag(group_state, GSTATE_CTX_LIST))
    return ListT;
  else if(check_flag(group_state, GSTATE_CTX_MAP))
    return MapT;
  else
    return GroupTUndef;
}

int8_t mk_group(struct ExprParserState *state, struct Expr *ex) {
  struct Expr **buf;

  struct Token *current = &state->src[*state->_i];
  struct Group *ghead = state->set_stack[state->set_ctr - 1];

  usize elements = ghead->delimiter_cnt + 1;

  enum GroupT group_ty = get_group_ty(ghead->state);
  
  if (group_ty == GroupTUndef || elements > state->expr_ctr) {
    throw_internal_error(state, "Internal group/operator stack overflowed.");
    return -1;
  }

  ex->type = LiteralExprT;
  ex->datatype = GroupT;
  ex->inner.value.literal.grouping.type = group_ty;
  ex->inner.value.literal.grouping.ptr = 0;
  ex->inner.value.literal.grouping.length = 0;

  if (!check_flag(ghead->state, GSTATE_EMPTY))
  {
    buf = calloc(elements, sizeof(void *));
    
    if (!buf)
      return -1;
    
    ex->inner.value.literal.grouping.ptr = buf;
    ex->inner.value.literal.grouping.length = elements;

    for (usize j = 0; elements > j; j++) {
      ex->inner.value.literal.grouping.ptr[j] = state->expr_stack[state->expr_ctr - 1];
      state->expr_ctr -= 1;
    }
  }

  return 0;
}

int8_t mk_idx_access(struct ExprParserState *state, struct Expr *ex) {
  ex->type = FnCallExprT;
  ex->datatype = 0;

  if (3 > state->expr_ctr) {
      throw_internal_error(state, "Not enough arguments on stack to create idx operation.");
      return -1;
  }

  ex->inner.idx.start   = state->expr_stack[state->expr_ctr - 1];
  ex->inner.idx.end     = state->expr_stack[state->expr_ctr - 2];
  ex->inner.idx.skip    = state->expr_stack[state->expr_ctr - 3];
  ex->inner.idx.operand = state->expr_stack[state->expr_ctr - 4];

  state->expr_ctr -= 4;
  return 0;
}

int8_t mk_fncall(struct ExprParserState *state, struct Expr *ex) {
  struct Token *current = &state->src[*state->_i];
  struct Group *ghead = state->set_stack[state->set_ctr - 1];
  usize argc = ghead->delimiter_cnt + 1;
  
  struct Expr *head;

  if (state->expr_ctr > state->expr_sz || 1 > *state->_i)
    return -1;
    
  ex->type = FnCallExprT;
  ex->datatype = 0;

  if (check_flag(ghead->state, GSTATE_EMPTY))
  {
    ex->inner.fncall.args = 0;
    ex->inner.fncall.args_length = 0;
  }
  else {
    ex->inner.fncall.args = calloc(argc, sizeof(void *));

    /* fill in arguments popping them off the stack */
    for (usize x=0; argc > x; x++){
      ex->inner.fncall.args[ex->inner.fncall.args_length] = head;
      ex->inner.fncall.args_length += 1;
      state->expr_ctr -= 1;
      head = state->expr_stack[state->expr_ctr - 1];
    }
  }
  
  // TODO
  // something feels off right here
  // like we're not pointing at the caller yet 
  head = state->expr_stack[state->expr_ctr - 1];
  state->expr_ctr -= 1;
  
  ex->inner.fncall.func_name = "anonymous";
  ex->inner.fncall.caller = head;
  
  if (head->type == SymExprT)
    ex->inner.fncall.func_name = head->inner.symbol;


  // /* this is usually the function name, but could be a grouping */
  
  // else if (head->type == LiteralExprT &&
  //          head->datatype == GroupT)
  //   nop;
  // else if (head->type == FnCallExprT)
  //   nop;
  // else
  //   /*expected a group, or symbol as last item on the stack*/
  //   return -1;

  return 0;
}

void mk_null(struct ExprParserState *state, struct Expr *ex) {
  ex->type = LiteralExprT;
  ex->datatype = NullT;
}

int8_t handle_idx_op(struct ExprParserState *state) {
  struct Expr ex;
  struct Group *ghead = state->set_stack[state->set_ctr - 1];
  struct Token *prev = &state->src[*state->_i - 1];

  if (ghead->origin->type != BRACE_OPEN)
      return -1;
    
  mk_null(state, &ex);
    
  /*
    peek-behind if token was COLON 
    add a value for its missing argument
  */
  // a: -> a:a
  // :: => ::a
  if (prev->type == COLON && 
    (add_dbg_sym(state, NULLTOKEN, 0) == -1 
    || inc_stack(state, &ex, false) == -1))
    return -1;
  
  // a:a -> a:a:a
  for (uint8_t i=0; 2 > ghead->delimiter_cnt; i++)
  {
    if(add_dbg_sym(state, NULLTOKEN, 0) == -1 
      || inc_stack(state, &ex, false) == -1)
        return -1;
  }

  memset(&ex, 0, sizeof(struct Expr));
  if (add_dbg_sym(state, _IdxAccess, 0) == -1
      || mk_idx_access(state, &ex) == -1
      || inc_stack(state, &ex, false) == -1)
        return -1;
  
  return 0;
}

int8_t handle_fncall(struct ExprParserState *state) {
  struct Group *ghead = state->set_stack[state->set_ctr - 1];
  struct Expr ex;

  if (ghead->origin->type != PARAM_OPEN 
      || add_dbg_sym(state, Apply, ghead->delimiter_cnt + 1) == -1
      || mk_fncall(state, &ex) == -1
      || inc_stack(state, &ex, false) == -1)
      return -1;
  return 0;
}

int8_t handle_grouping(struct ExprParserState *state) {
  enum Lexicon marker_type;
  struct Group *ghead = state->set_stack[state->set_ctr - 1];
  struct Token *current = &state->src[*state->_i];
  struct Expr ex;

  marker_type = get_dbg_group_token(current->type);
  if (marker_type == TOKEN_UNDEFINED 
    || add_dbg_sym(state, marker_type, ghead->delimiter_cnt + 1) == -1
    || mk_group(state, &ex) == -1
    || inc_stack(state, &ex, false) == -1)
    return -1;
  
  return 0;
}

int8_t handle_close_brace(struct ExprParserState *state) {
  struct Token *prev = 0, *ophead=0, *current = &state->src[*state->_i];
  struct Group *ghead;
  struct Expr ex;
  int8_t ret = 0;

  enum Lexicon marker_type = TOKEN_UNDEFINED;
  
  if (state->set_ctr == 0) {
    mk_error(state, Fatal, "Unexpected closing brace.");
    return -1;
  }
  /* Operators stack is empty */
  else if (
      0 > state->operators_ctr
      /* unbalanced brace, extra closing brace.*/
      || state->set_ctr == 0
      || state->set_ctr > state->set_sz)
    return -1;

  if (*state->_i > 0)
    prev = &state->src[*state->_i - 1];
  else
    return -1;

  /* Grab the head of the group stack & decrement */
  ghead = state->set_stack[state->set_ctr - 1];
  state->set_ctr -= 1;

  if (prev->type == invert_brace_tok_ty(current->type)) {
    set_flag(&ghead->state, GSTATE_EMPTY);
    state->operators_ctr -= 1;
    
    if (check_flag(ghead->state, GSTATE_OP_IDX)) {
      mk_error(state, Error, "Slice must contain atleast one delimiter or value.");
      return -1;
    }    
  }

  else {
    if (flush_ops_until_br(state) == -1)
      return -1;
    
    /* Grab the head of the operator stack */
    // reach into de-allocated space 
    // and just check for sanity purposes
    // that our current group's origin
    // is this group
    ophead = state->operator_stack[state->operators_ctr];

    if (ophead != ghead->origin) {
      throw_internal_error(state, "shit man.");
      return -1;
    }
  }

  if (check_flag(ghead->state, GSTATE_OP_IDX))
    ret = handle_idx_op(state);
  
  else if (check_flag(ghead->state,  GSTATE_OP_APPLY))
    ret = handle_fncall(state);
  
  else if (check_flag(ghead->state, GSTATE_OP_GROUP))
    ret = handle_grouping(state);
  
  else return -1;

  return ret;
}

int8_t handle_delimiter(struct ExprParserState *state) {
  struct Expr ex;
  struct Token *head = 0, 
    *current = &state->src[*state->_i],
    *prev = 0,
    *next = 0;

  struct Group *ghead = 0;

  /* Setup group group head ptr */
  if (state->set_ctr > 0)
    ghead = state->set_stack[state->set_ctr - 1];
  else
  { 
    /* they didn't add an opening brace*/
    mk_error(state, Error, "delimiters must be used inside of a nesting.");
    return -1;
  }
    
  ghead->delimiter_cnt += 1;
  ghead->last_delim = current;
  
  if(*state->_i > 0)
    prev = &state->src[*state->_i - 1];
  
  if(state->src_sz > *state->_i + 1)
    next = &state->src[*state->_i + 1];

  if (!next || !prev)
  {
    mk_error(state, Fatal, "Expected token before & after delimiter.");
    return -1;
  }

  /* 
    Tuple/List/IDX/APPLY operations will already have the context locked.
  */
  if (!check_flag(ghead->state, GSTATE_CTX_LOCK))
  {
    /*
      upgrade CTX_SET into CTX_MAP & lock the context
    */
    if (check_flag(ghead->state, GSTATE_CTX_SET))
    {
      if (current->type == COLON)
      {
        set_flag(&ghead->state, GSTATE_CTX_MAP | GSTATE_CTX_LOCK);
        unset_flag(&ghead->state, GSTATE_CTX_SET);
      } 

      else if (current->type == COMMA)
        set_flag(&ghead->state, GSTATE_CTX_LOCK);
      
      else
      {
        throw_internal_error(state, "Unexpected token.");
        return -1;
      }
    }
  }
  /* peek behind and add NULL if no */
  if (check_flag(ghead->state, GSTATE_CTX_IDX))
  {
    if(ghead->delimiter_cnt > 2)
    {
      throw_internal_error(state, "Not enough items to create idx operation.");
      return -1;
    }

    /* add NOP to the output if no position argument present*/
    if (prev == ghead->origin || prev->type == COLON) 
    { 
      mk_null(state, &ex);
      if (add_dbg_sym(state, NULLTOKEN, 0) == -1
         ||inc_stack(state, &ex, false))
        return -1;
    }
  }

  if (flush_ops_until_br(state) == -1)
    return -1;
  
  return 0;
}

int8_t handle_str(struct ExprParserState *state) {
  struct Expr ex;
  char * str;
  struct Token *current = &state->src[*state->_i];
  usize size = current->end - current->start;

  ex.type = LiteralExprT;
  ex.datatype = StringT;
  memcpy(&ex.origin, current, sizeof(struct Token));

  str = malloc(size+1);
  if (str == 0) {
    throw_internal_error(state, "Allocation failure.");
    return -1;
  }
  
  ex.inner.value.literal.string = str;

  memcpy(
    ex.inner.value.literal.string,
    &state->line[current->start],
    size
  );

  ex.inner.value.literal.string[size] = 0;
  
  if(inc_stack(state, &ex, true) == -1)
    return -1;
  
  return 0;
}


// int8_t unwind_operator(struct ExprParserState *state, struct Expr *ex) {
//   struct Expr *op = state->expr_stack[state->expr_ctr - 1];

//   state->expr_stack[state->expr_ctr - 1] = ex->inner.bin.lhs;
//   state->expr_stack[state->expr_ctr] = ex->inner.bin.rhs;
//   state->expr_ctr += 1;
//   //struct Token *current = &state->src[*state->i - 1];
// }


// int8_t handle_unwind(struct ExprParserState *state) {
//   struct Token *prev = &state->src[*state->i - 1],
//               *start, *end;
//   struct Group *ghead = 0;
   
//   if (state->set_ctr > 0)
//     ghead = &state->set_stack[state->set_ctr - 1];
//   else
//    return -1;

//   //struct Token *spool_head = state->debug[*state->debug_ctr - 1];
  
//   if (ghead->last_delim != 0)
//     start = ghead->last_delim;

//   else if (ghead->origin != 0)
//     start = ghead->origin;

//   else {}

  // for (usize i = *state->i; state->src_sz > i; i++)
  // {

  //   if (state->src[i].type == COLON || state->src[i].type == COMMA)
  //   {
      
  //   }
  // }
//   return 0;
// }

int8_t initalize_parser_state(
    char * line,
    struct Token tokens[],
    usize ntokens,
    usize *i,
    struct ExprParserState *state
  )
{
  memset(state, 0, sizeof(struct ExprParserState));
  if (init_vec(&state->expr_pool, 2048, sizeof(struct Expr)) == -1
    ||init_vec(&state->pool, 256, sizeof(struct Token)) == -1
    ||init_vec(&state->debug, 2048, sizeof(void *)) == -1
    ||init_vec(&state->errors, 64, sizeof(struct CompileTimeError)) == -1)
      return -1;
  
  state->line = line;
  state->_i = i;

  state->src = tokens;
  state->src_sz = ntokens;

  state->expr_ctr = 0;
  state->expr_sz = STACK_SZ;

  state->set_ctr = 0;
  state->set_sz = STACK_SZ;

  state->operators_ctr = 0;
  state->operator_stack_sz = STACK_SZ;
  state->panic_flags = 0;
  state->expecting = 0;

  state->canary_expr = 0;
  state->canary_set = 0;
  state->canary_op = 0;
  
  set_flag(&state->expecting,  EXPECTING_OPEN_PARAM 
    | EXPECTING_STRING | EXPECTING_SYMBOL | EXPECTING_INTEGER
    | EXPECTING_OPEN_BRACKET 
    | EXPECTING_OPEN_BRACE
    | EXPECTING_NEXT
  );
  
  return 0;
}

int8_t is_canary_dead(struct ExprParserState *state) {
  return state->canary_expr != 0 || state->canary_op != 0 || state->canary_set != 0;
}

int8_t parse_expr(
    char * line,
    struct Token tokens[],
    usize expr_size,
    struct ExprParserState *state,
    struct Expr *ret
){
  usize i = 0;

  if (initalize_parser_state(line, tokens, expr_size, &i, state) == -1)
    return -1;

  for (i = 0; expr_size > i; i++) {
    if (state->expr_ctr > state->expr_sz
        || state->operators_ctr > state->operator_stack_sz
        || is_canary_dead(state))
      return -1;

    else if (check_flag(state->panic_flags, STATE_PANIC) 
      || is_token_unexpected(&tokens[i], state->set_stack[state->set_ctr - 1], state->expecting)) {
      // TODO
      // handle_unwind(&state);
      return -1;
    }
    
    else if(state->src[i].type == WORD)
      handle_symbol(state);

    else if(state->src[i].type == INTEGER)
      handle_int(state);
    
    else if(state->src[i].type == STRING_LITERAL)
      handle_str(state);

    else if (is_operator(state->src[i].type))
      handle_operator(state);

    else if (is_close_brace(state->src[i].type))
      handle_open_brace(state);

    else if (is_open_brace(state->src[i].type))
      handle_open_brace(state);

    else if (state->src[i].type == COLON || state->src[i].type == COMMA)
      handle_delimiter(state);

    else if (state->src[i].type == IF)
      //todo
      nop;
    
    else if (state->src[i].type == ELSE)
      //todo
      // handle `else if` here aswell
      nop;

    else if (state->src[i].type == RETURN)
     //todo
     nop;

    else {
#ifdef DEBUG
      printf("debug: token fell through precedense [%s]\n",
             ptoken(tokens[i].type));
#endif
    }
    /* setup next-token expectation */
    state->expecting = expecting_next(tokens[i].type);
  }
  
  /* dump the remaining operators onto the output */
  if (flush_all_ops(state) == -1
    || state->expr_ctr != 1)
    return -1;
  
  ret = ((struct Expr **)(state->expr_pool.base))[0];

  return 0;
}

int8_t free_state(struct ExprParserState *state) {
  
  if (vec_free(&state->expr_pool) == -1
  || vec_free(&state->debug) == -1
  || vec_free(&state->pool) == -1
  || vec_free(&state->errors) == -1)
    return -1;
  
  return 0;
}

int8_t reset_state(struct ExprParserState *state) {
  
  memset(state->expr_stack, 0, sizeof(void *[STACK_SZ]));
  state->expr_ctr = 0;
  
  memset(state->operator_stack, 0, sizeof(void *[STACK_SZ]));
  state->operators_ctr = 0;
  
  memset(state->set_stack, 0, sizeof(void *[STACK_SZ]));
  state->set_ctr = 0;
  
  state->src_sz = 0;
  state->src = 0;
  state->_i = 0;
  state->line = 0;
  
  state->expecting = 0;
  state->panic_flags = 0;
  
  free_state(state);
  return 0;
}

// int8_t new_expr(char *line, struct Token tokens[], usize ntokens, struct Expr *expr) {
//     /*todo: create expr tree*/
//     return 0;
// }
