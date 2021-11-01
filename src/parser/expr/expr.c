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

#define OPERATOR_BUF_SZ 64
#define END_PRECEDENCE 127

typedef uint16_t FLAG_T; 

#define GSTATE_EMPTY       2
#define GSTATE_CTX_LIST    4
#define GSTATE_CTX_TUPLE   8
#define GSTATE_CTX_SET    16
#define GSTATE_CTX_MAP    32
#define GSTATE_CTX_IDX    64
#define GSTATE_CTX_LOCK  128
#define GSTATE_OP_IDX    256
#define GSTATE_OP_APPLY  512
#define GSTATE_OP_GROUP 1024

#define FLAG_ERROR           65535
#define EXPECTING_SYMBOL         2
#define EXPECTING_INTEGER        4
#define EXPECTING_STRING         8
#define EXPECTING_ARITHMETIC_OP 16

/* . */
#define EXPECTING_APPLY_OPERATOR 32
#define EXPECTING_OPEN_BRACKET   64   
#define EXPECTING_CLOSE_BRACKET  128
#define EXPECTING_OPEN_PARAM     256    
#define EXPECTING_CLOSE_PARAM    512
#define EXPECTING_OPEN_BRACE     1024
#define EXPECTING_CLOSE_BRACE    2048

#define EXPECTING_NEXT           4096 
/* : , */
#define EXPECTING_DELIMITER      8192

#define STATE_INCOMPLETE          1
#define STATE_PANIC               2 
#define INTERNAL_ERROR            4

/* if set - warning messages are present */
#define STATE_WARNING             8

#define STACK_SZ 128
#define _OP_SZ 128

/*
    The grouping stack is used to track the amount 
    of sub-expressions inside an expression. (See lexer.h)
    We generate GROUPING tokens based on the stack model
    our parser uses.

    For every new brace token-type added into the operator-stack
    increment the grouping stack, and initalize it to 0.
    For every comma, increment the current grouping-stack's head by 1.
    
    Once the closing brace is found and
    this stack's head is larger than 0,
    we have a set/grouping of expressions. 
  
    used in the group-stack exclusively
*/
struct Group {
    // amount of delimiters
    uint16_t delimiter_cnt;

    /*
        0 : Uninitialized state
        1 : COLON token in group
        2 : COMMA token in group
        4 : List context mode
        8 : Tuple context mode
       16 : Set context mode
       32 : Map context mode
       64 : Index context mode
      (?) : Tuple context mode (without bracing)
      128 : lock context mode
      256 : index marker operation
      512 : apply marker operation
     1024 : group marker operation
    */
    FLAG_T state;

    // should be `[` `(` '{' or `0`
    struct Token *origin;
    struct Token *last_delim;
};

struct ExprParserState {
    struct Token *src;
    usize src_sz;
    usize *i;
    char * line;

    struct Token **debug;
    usize debug_sz;
    usize *debug_ctr;

    struct Expr **expr_stack;
    usize expr_ctr;
    usize expr_sz;

    struct Vec *expr_pool;

    struct Token *pool;
    usize pool_i;
    usize pool_sz;

    struct Group *set_stack;
    usize set_ctr;
    usize set_sz;

    struct Token **operator_stack;
    uint16_t operators_ctr;
    uint16_t operator_stack_sz;

    struct CompileTimeError *errors;
    uint16_t errors_ctr;

    FLAG_T expecting;
    FLAG_T state;
};

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
  return 0 | EXPECTING_ARITHMETIC_OP
    | EXPECTING_APPLY_OPERATOR;
}

FLAG_T expect_opposite_brace(enum Lexicon brace){
  switch (brace) {
    case PARAM_OPEN: return EXPECTING_CLOSE_PARAM;
    case BRACE_OPEN: return EXPECTING_CLOSE_BRACE;
    case BRACKET_OPEN: return EXPECTING_CLOSE_BRACKET;
    default: return 0;
  } 
}

void mk_error(struct ExprParserState *state, enum ErrorT type, const char * msg) {
  struct CompileTimeError *err;
  
  err->type = type;
  state->errors[state->errors_ctr].base = &state->src[*state->i];
  state->errors_ctr += 1;
  err->msg = msg;

  set_flag(&state->state, STATE_PANIC | STATE_INCOMPLETE);
}

void throw_internal_error(struct ExprParserState *state, const char * meta, const char * msg)
{
  char * internal_msg;
  struct CompileTimeError *err = &state->errors[state->errors_ctr];

#ifdef DEBUG
  internal_msg = malloc(strlen(meta) + strlen(msg));
  strcat(internal_msg, meta);
  strcat(internal_msg, msg);
#else
  internal_msg = msg;
#endif
  err->free_msg = true;
  err->msg = internal_msg;
  err->type = Fatal;
  set_flag(&state->state, STATE_PANIC | STATE_INCOMPLETE | INTERNAL_ERROR);
  mk_error(state, Fatal, internal_msg);
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
      "!"       : 2 R
      "!= == >= > <= < && ||": 2 L
      "+= -= =" : 1 R
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
    
    else if (token == EQUAL
      || token == PLUSEQ
      || token == MINUSEQ)
      return 1;
    
    else if (is_open_brace(token))
        return 0;
    
    return -1;
}

int8_t add_marker(
  struct ExprParserState *state,
  enum MarkerT type,
  usize argc
){
  struct Token *ret;
  if (state->pool_i > state->pool_sz)
    return -1;
  
  state->pool[state->pool_i].type = MARKER;
  state->pool[state->pool_i].start = type;
  state->pool[state->pool_i].end = argc;

  ret = &state->pool[state->pool_i];
  state->pool_i += 1;

  if (*state->debug_ctr > state->debug_sz)
  {
    throw_internal_error(state, "debug_ctr overflowed.");
    return -1;
  }

  state->debug[*state->debug_ctr] = ret;
  *state->debug_ctr += 1;

  return 0;
}

enum Lexicon get_expected_delimiter(struct ExprParserState *state) {
  /* setup delimiter expectation */
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];

  if(ghead->state == 0
    ||check_flag(GSTATE_CTX_SET, ghead->state)
    ||check_flag(GSTATE_CTX_LIST, ghead->state)
    ||check_flag(GSTATE_CTX_TUPLE, ghead->state)
  ) return COMMA;
  
  else if (check_flag(GSTATE_CTX_IDX, ghead->state))
    return COLON;
  
  else if (check_flag(GSTATE_CTX_MAP, ghead->state)) {
    if (ghead->delimiter_cnt % 2 == 0)
      return COMMA;
    else
      return COLON;
    // 2:3, 3:4
  }

  else
    return 0;
}

int8_t is_token_unexpected(struct ExprParserState *state) {
  struct Token * current = &state->src[*state->i];
  enum Lexicon delim;
  FLAG_T check_list = 0;

  if (current->type == WORD || current->type == NULLTOKEN)
    set_flag(&check_list, EXPECTING_SYMBOL);

  else if (current->type == DOT)
    set_flag(&check_list, EXPECTING_APPLY_OPERATOR);

  else if (is_operator(current->type))
    set_flag(&check_list, EXPECTING_ARITHMETIC_OP);
  
  else if (is_delimiter(current->type)) {
    delim = get_expected_delimiter(state);
    
    if (delim != current->type)
      return -1;

    set_flag(&check_list, EXPECTING_DELIMITER);
  }

  else {  
    switch (current->type) {
      case INTEGER:
        set_flag(&check_list, EXPECTING_INTEGER);
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

  return !check_flag(state->state, check_list);
}

FLAG_T setup_flags(struct ExprParserState* state)
{
  struct Token *current = &state->src[*state->i];
  FLAG_T ret = FLAG_ERROR;

  if (is_symbolic_data(current->type))
  {
    set_flag(&ret, 0
      | EXPECTING_ARITHMETIC_OP      
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

  else if (is_operator(current->type)) {
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
      if (expect_opposite_brace(current->type) == 0)
      {
        throw_internal_error(state, "Unexpected return. (0)");
        return -1;

      }
      
      set_flag(&ret, 0
        | expect_any_open_brace()
        | expect_any_data()
        | expect_opposite_brace(current->type)
        | EXPECTING_NEXT
      );
    }

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

int8_t inc_stack(
  struct ExprParserState *state,
  struct Expr *ex,
  bool add_debug)
{
  struct Expr * vec_item;
  
  if (state->expr_ctr > state->expr_sz || *state->debug_ctr > state->debug_sz)
  {
    throw_internal_error(state, "Expr/debug ctr overflowed.");
    return -1;
  }
  
  vec_item = vec_push(state->expr_pool, &ex);

  if (vec_item == 0) {
    throw_internal_error(state, "vec pool returned null ptr.");
    return -1;
  }
  
  state->expr_stack[state->expr_ctr] = vec_item;
  state->expr_ctr += 1;

  state->debug[*state->debug_ctr] = &state->src[*state->i];
  *state->debug_ctr += 1;

  return 0;
}

int8_t handle_int(struct ExprParserState* state)
{
  struct Expr ex;
  struct Token *current = &state->src[*state->i];
  usize size = current->end - current->start;

  char * end;
  ex.type = LiteralExprT;
  ex.datatype = IntT;
  errno = 0;

  ex.inner.value.literal.integer =
    str_to_isize(state->line + current->start, &end, 10);

  if (errno != 0)
    return -1;

  else if (end != state->line + size) {
    throw_internal_error(state, "Didn't parse integer correctly");
    return -1;
  }

  if (inc_stack(state, &ex, true) == -1)
    return -1;
  
  return 0;
}

/*
  symbols are placed directly into the output.
*/
int8_t handle_symbol(struct ExprParserState* state) {
  struct Expr ex;
  struct Token *current = &state->src[*state->i];
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
  struct Token *current = &state->src[*state->i];

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
int8_t flush_ops(struct ExprParserState *state)
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
    if (head->type == invert_brace_tok_ty(state->src[*state->i].type))
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

int8_t handle_operator(struct ExprParserState *state) {
  struct Expr ex;
  struct Token *head=0, *current=0;
  int8_t precedense = 0, head_precedense = 0;

  current = &state->src[*state->i];
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
  
  state->operator_stack[state->operators_ctr] = &state->src[*state->i];
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

  current = &state->src[*state->i];

  if (*state->i > 0)
    prev = &state->src[*state->i - 1];
  
  if (state->src_sz - 1 > *state->i)
    next = &state->src[*state->i + 1];

  /* overflow check */
  if (state->set_ctr > state->set_sz ||
      state->operators_ctr > state->operator_stack_sz)
  {
    throw_internal_error(state, "Internal group/operator stack overflowed.");
    return -1;
  }
  
  ghead = &state->set_stack[state->set_ctr];
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

    if (prev && (is_close_brace(prev->type)
       || prev->type == WORD))
          set_flag(&ghead->state, GSTATE_OP_APPLY);
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
  else if (current->type == BRACE_OPEN)
    ghead->state = 0 | GSTATE_CTX_SET;
  
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

  struct Token *current = &state->src[*state->i];
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];

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
  struct Token *current = &state->src[*state->i];
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  usize argc = ghead->delimiter_cnt + 1;
  
  struct Expr *head;

  if (state->expr_ctr > state->expr_sz || 1 > *state->i)
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
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  struct Token *prev = &state->src[*state->i - 1];

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
    (add_marker(state, MarkerNop, 0) == -1 
    || inc_stack(state, &ex, false) == -1))
    return -1;
  
  // a:a -> a:a:a
  for (uint8_t i=0; 2 > ghead->delimiter_cnt; i++)
  {
    if(add_marker(state, MarkerNop, 0) == -1 
      || inc_stack(state, &ex, false) == -1)
        return -1;
  }

  memset(&ex, 0, sizeof(struct Expr));
  if (add_marker(state, _IdxAccess, 0) == -1
      || mk_idx_access(state, &ex) == -1
      || inc_stack(state, &ex, false) == -1)
        return -1;
  
  return 0;
}

int8_t handle_fncall(struct ExprParserState *state) {
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  struct Expr ex;

  if (ghead->origin->type != PARAM_OPEN 
      || *state->debug_ctr > state->debug_sz
      || add_marker(state, Apply, ghead->delimiter_cnt + 1) == -1
      || mk_fncall(state, &ex) == -1
      || inc_stack(state, &ex, false) == -1)
      return -1;
  return 0;
}

int8_t handle_grouping(struct ExprParserState *state) {
  enum MarkerT marker_type;
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  struct Token *current = &state->src[*state->i];
  struct Expr ex;

  marker_type = derive_group_marker_t(current->type);
  if (marker_type == MarkerTUndef 
    || add_marker(state, marker_type, ghead->delimiter_cnt + 1) == -1
    || mk_group(state, &ex) == -1
    || inc_stack(state, &ex, false) == -1)
    return -1;
  
  return 0;
}

int8_t handle_close_brace(struct ExprParserState *state) {
  struct Token *prev = 0, *ophead=0, *current = &state->src[*state->i];
  struct Group *ghead;
  struct Expr ex;
  int8_t ret = 0;

  enum MarkerT marker_type = MarkerNop;
  
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

  if (*state->i > 0)
    prev = &state->src[*state->i - 1];
  else
    return -1;

  /* Grab the head of the group stack & decrement */
  ghead = &state->set_stack[state->set_ctr - 1];
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
    if (flush_ops(state) == -1)
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
    *current = &state->src[*state->i],
    *prev = 0,
    *next = 0;

  struct Group *ghead = 0;

  /* Setup group group head ptr */
  if (state->set_ctr > 0)
    ghead = &state->set_stack[state->set_ctr - 1];
  else
  { 
    /* they didn't add an opening brace*/
    mk_error(state, Error, "delimiters must be used inside of a nesting.");
    return -1;
  }
    
  ghead->delimiter_cnt += 1;
  ghead->last_delim = current;
  
  if(*state->i > 0)
    prev = &state->src[*state->i - 1];
  
  if(state->src_sz > *state->i + 1)
    next = &state->src[*state->i + 1];

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
      if (add_marker(state, MarkerNop, 0) == -1
         ||inc_stack(state, &ex, false))
        return -1;
    }
  }

  if (flush_ops(state) == -1)
    return -1;
  
  return 0;
}

int8_t handle_str(struct ExprParserState *state) {
  struct Expr ex;
  char * str;
  struct Token *current = &state->src[*state->i];
  usize size = current->end - current->start;

  ex.type = LiteralExprT;
  ex.datatype = StringT;

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


int8_t handle_unwind(struct ExprParserState *state) {
  struct Token *current = &state->src[*state->i];
  struct Group *ghead = &state->set_stack[state->set_ctr];

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

    infix: (1+2) * (1 + 1)
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

int8_t parse_expr(
    struct Token tokens[], usize expr_size,
    struct Token *output[], usize output_sz,
    usize *output_ctr,
    struct Token token_pool[],
    usize pool_sz,
    struct CompileTimeError *err)
{
  struct ExprParserState state;  
  struct Token *head, *operators[STACK_SZ];
  struct Group groups[STACK_SZ];
  
  struct Expr ex;

  int8_t ret = -1;
  int8_t precedense = 0;
  usize i = 0;

  state.i = &i;
  state.src = tokens;
  state.src_sz = expr_size;

  state.debug = output;
  state.debug_ctr = output_ctr;
  state.debug_sz = output_sz;

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

    else if (check_flag(state.state, STATE_PANIC) || is_token_unexpected(&state))
      handle_unwind(&state);
    
    else if(state.src[i].type == WORD)
      handle_symbol(&state);

    else if(state.src[i].type == INTEGER)
      handle_int(&state);
    
    else if(state.src[i].type == STRING_LITERAL)
      handle_str(&state);

    else if (is_operator(state.src[i].type))
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
             ptoken(tokens[i].type));
#endif
    }
    /* setup next-token expectation */
    state.expecting = setup_flags(&state);
  }
  
  /* dump the remaining operators onto the output */
  while (state.operators_ctr > 0) {
    memset(&ex, 0, sizeof(struct Expr));
    head = operators[state.operators_ctr - 1];
    /*
        any remaining params/brackets/braces are unclosed
        indiciate invalid expressions
    */
    if (*state.debug_ctr > state.debug_sz || is_open_brace(head->type))
      return -1;

    if (mk_operator(&state, &ex, head) == -1
        || inc_stack(&state, &ex, true) == -1)
        return -1;
    
    state.operators_ctr -= 1;
  }

  return 0;
}

// int8_t new_expr(char *line, struct Token tokens[], usize ntokens, struct Expr *expr) {
//     /*todo: create expr tree*/
//     return 0;
// }
