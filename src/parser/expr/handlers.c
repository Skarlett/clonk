#include "handlers.h"
#include "expr.h"
#include <string.h>
#include <errno.h>

/* no special algorithms, just an equality test */
void determine_return_ty(struct Expr *bin) {
  if (bin->inner.bin.rhs->datatype == bin->inner.bin.lhs->datatype)
    bin->inner.bin.returns = bin->inner.bin.rhs->datatype;
  else
    bin->inner.bin.returns = UndefT;
}

void mk_null(struct Expr *ex) {
  ex->type = NopExprT;
  ex->datatype = NullT;
}

/*
  integers are placed directly into the output.
*/
int8_t mk_int(struct ExprParserState* state, struct Expr *ex)
{
  struct Token *current = &state->src[*state->_i];
  char * _; 
  
  ex->type = LiteralExprT;
  ex->datatype = IntT;
  errno = 0;

  ex->inner.value.literal.integer =
    str_to_isize(state->line + current->start, &_, 10);

  if (errno != 0) {
    throw_internal_error(state, "Didn't convert integer correctly");
    return -1;
  }
 
  return 0;
}

/*
  symbols are placed directly into the output.
*/
int8_t mk_symbol(struct ExprParserState* state, struct Expr *ex) 
{
  struct Token *current = &state->src[*state->_i];
  uint8_t size = current->end - current->start;
  
  ex->type = SymExprT;
  ex->datatype = UndefT;
  ex->inner.symbol = calloc(1, size+1);
  
  /*bad alloc*/
  if (ex->inner.symbol == 0) {
    throw_internal_error(state,  "Allocation failure");
    return -1;
  }
    
  memcpy(ex->inner.symbol,
    state->line + current->start,
    size + 1
  );
  
  ex->inner.symbol[size] = 0;
  return 0;
}

/*
  strings are placed directly into the output.
*/
int8_t mk_str(struct ExprParserState *state, struct Expr *ex) {
  char * str;
  struct Token *current = &state->src[*state->_i];
  uint16_t size = current->end - current->start;

  ex->type = LiteralExprT;
  ex->datatype = StringT;
  memcpy(&ex->origin, current, sizeof(struct Token));

  str = malloc(size+1);
  if (str == 0) {
    throw_internal_error(state, "Allocation failure.");
    return -1;
  }
  
  ex->inner.value.literal.string = str;

  memcpy(
    ex->inner.value.literal.string,
    &state->line[current->start],
    size
  );

  ex->inner.value.literal.string[size] = 0;
  return 0;
}

int8_t mk_group(struct ExprParserState *state, struct Expr *ex) {
  struct Expr **buf;
  struct Token *current = &state->src[*state->_i];
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];

  uint16_t elements = ghead->delimiter_cnt + 1;

  enum GroupT group_ty = get_group_ty(ghead);
  
  if (group_ty == GroupTUndef || elements > state->expr_ctr) {
    throw_internal_error(state, "Internal group/operator stack overflowed.");
    return -1;
  }

  ex->type = LiteralExprT;
  ex->datatype = GroupT;
  ex->inner.value.literal.grouping.type = group_ty;
  ex->inner.value.literal.grouping.ptr = 0;
  ex->inner.value.literal.grouping.length = 0;

  if (ghead->state & GSTATE_EMPTY == 0)
  {
    buf = calloc(elements, sizeof(void *));
    
    if (!buf)
      return -1;
    
    ex->inner.value.literal.grouping.ptr = buf;
    ex->inner.value.literal.grouping.length = elements;

    for (uint16_t j = 0; elements > j; j++) {
      ex->inner.value.literal.grouping.ptr[j] = state->expr_stack[state->expr_ctr - 1];
      state->expr_ctr -= 1;
    }
  }

  return 0;
}

enum Operation operation_from_token(enum Lexicon t)
{
    switch (t) {
        /* arithmetic */
	case ADD: return Add;
        case SUB: return Sub;
        case MUL: return Multiply;
        case DIV: return Divide;
        case MOD: return Modolus;
        case POW: return Pow;
	/* boolean logic */
        case AND: return And;
        case OR: return Or;
	/* case NOT: return Not; */
	/* comparison */
        case ISEQL: return IsEq;
        case ISNEQL: return NotEq;
        case GTEQ: return GtEq;
        case LTEQ: return LtEq;
        case LT: return Lt;
        case GT: return Gt;
	/* access operator */
        case DOT: return Access;
	/* assignments */
        case EQUAL: return Assign;
        case PLUSEQ: return AssignAdd;
        case MINUSEQ: return AssignSub;
	/* bitwise assignments */
	case BANDEQL: return BandEql;
	case BOREQL: return BorEql;
	case BNEQL: return BnotEql;
	/* bitwise boolean logic */
	case PIPE: return BitOr;
	case AMPER: return BitAnd;
	case TILDE: return BitNot;
	/* bit shifting */
	case SHL: return ShiftLeft;
	case SHR: return ShiftRight;
        default: return UndefinedOp;
    }
}


int8_t mk_binop(struct Token *operator, struct ExprParserState *state, struct Expr *ex) { 
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
  
  ex->inner.bin.op = operation_from_token(operator->type);
  if (ex->inner.bin.op == UndefinedOp)
    return -1;
  
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

int8_t mk_operator(struct ExprParserState *state, struct Expr *ex, struct Token *op_head)
{
  if (op_head->type == NOT && mk_not(state, ex) == -1
     || mk_binop(op_head, state, ex) == -1)
     return -1;
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

/* 
 * Apply operations pop N+1 arguments from the stack where 
 * N is derived from `struct Group`'s `delmiter_ctr` + 1.
 *
 * `delimiter_ctr + 1` representing the total number of arguments.
 * `N + 1` representing the total arguments and the function name
 *
 * Noteworthy: foo()() will produce a top level APPLY expression, 
 * with a nested APPLY expression
 * 
 * src: name(args, ...) 
 * dbg: <name> <args> ... Apply
 *
 */
int8_t mk_fncall(struct ExprParserState *state, struct Expr *ex) {
  struct Token *current = &state->src[*state->_i];
  struct Group *ghead = &state->set_stack[state->set_ctr - 1];
  uint16_t argc = ghead->delimiter_cnt + 1;
  
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
    for (uint16_t x=0; argc > x; x++){
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

int8_t mk_if_cond(struct ExprParserState *state, struct Expr *ex)
{
  struct Expr *exhead = 0;
  
  if (1 > state->expr_ctr)
    return -1;

  exhead = state->expr_stack[state->expr_ctr - 1];

  ex->type = IfExprT;
  ex->inner.cond.cond = exhead;

  return 0;
}

int8_t mk_if_body(struct ExprParserState *state, struct Expr *ex)
{
  struct Expr *body, *cond = 0;
  
  if (2 > state->expr_ctr)
    return -1;

  cond = state->expr_stack[state->expr_ctr - 2];
  body = state->expr_stack[state->expr_ctr - 1];
  
  if (cond->type != IfExprT)
    return -1;
  
  cond->inner.cond.body = body;
  state->expr_ctr -= 1;

  return 0;
}

int8_t mk_else_body(struct ExprParserState *state, struct Expr *ex)
{
  struct Expr *body, *cond = 0;
  
  if (2 > state->expr_ctr)
    return -1;

  cond = state->expr_stack[state->expr_ctr - 2];
  body = state->expr_stack[state->expr_ctr - 1];
  
  if (cond->type != IfExprT)
    return -1;
  
  cond->inner.cond.else_body = body;
  state->expr_ctr -= 1;

  return 0;
}


