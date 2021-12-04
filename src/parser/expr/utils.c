
#include <string.h>
#include <assert.h>
#include "expr.h"
#include "utils.h"
#include "../../utils/vec.h"

int8_t add_dbg_sym(
  struct ExprParserState *state,
  enum Lexicon type,
  uint16_t argc
){
  struct Token marker, *ret;
  assert(type != TOKEN_UNDEFINED);

  marker.type = type;
  marker.start = 0;
  marker.end = argc;
  
  // push debug token in the pool
  ret = vec_push(&state->pool, &marker);
  assert(ret != 0);
  assert(vec_push(&state->debug, &ret) != 0);
  return 0;
}

int8_t inc_stack(
  struct ExprParserState *state,
  struct Expr *ex,
  struct Token *dbg_out
){
  struct Expr * heap_ex;
  assert(state->expr_ctr < state->expr_sz);
  
  heap_ex = vec_push(&state->expr_pool, &ex);
  assert(heap_ex != 0);
  
  if (dbg_out)
    assert(vec_push(&state->debug, &dbg_out) != 0);
  
  state->expr_stack[state->expr_ctr] = heap_ex;
  state->expr_ctr += 1;

  return 0;
}

struct Token * prev_token(struct ExprParserState *state) 
{
  if (*state->_i != 0)
    return &state->src[*state->_i - 1];
  return 0;
}

struct Token * next_token(struct ExprParserState *state) 
{
  if(UINT16_MAX > *state->_i && state->src_sz > *state->_i)
    return &state->src[*state->_i + 1];
  return 0;
}

struct Group * group_head(struct ExprParserState *state)
{
  if (STACK_SZ - 1 > state->set_ctr && state->set_ctr > 0)
    return &state->set_stack[state->set_ctr - 1];
  return 0;
}

struct Token * op_head(struct ExprParserState *state)
{
  if(state->operators_ctr > 0)
    return state->operator_stack[state->operators_ctr - 1];
  return 0;
}


bool is_op_keyword(enum Lexicon token) 
{
  return token == IF
     || token == ELSE
     || token == RETURN
     || token == FUNC_DEF
     /* internal */
     || token == IfCond
     || token == IfBody
     || token == DefSign
     || token == DefBody;
}

struct Group * new_grp(struct ExprParserState *state, struct Token * origin) 
{
  struct Group *ghead;
  assert(state->set_ctr < state->set_sz);
  
  ghead = &state->set_stack[state->set_ctr];
  ghead->operator_idx = state->set_ctr;
  state->set_ctr += 1;

  ghead->delimiter_cnt = 0;
  ghead->origin = origin;
  
  ghead->state = GSTATE_CTX_DATA_GRP; 
  return ghead;
}


struct Token * op_push(enum Lexicon op, uint16_t start, uint16_t end, struct ExprParserState *state)
{
  struct Token new, *heap = 0;
  new.type = op;
  new.end = end;
  new.start = start;

  heap = vec_push(&state->pool, &new);
  assert(heap);
  
  state->operator_stack[state->operators_ctr] = heap;  
  state->operators_ctr += 1;
  return heap;
}

enum Lexicon grp_dbg_sym(enum GroupT type)
{
  switch (type) {
    case ListT: return ListGroup;
    case SetT: return SetGroup;
    case MapT: return MapGroup;
    case CodeBlockT: return CodeBlock;
    case TupleT: return TupleGroup;
    default: return TOKEN_UNDEFINED;
  };
}

int8_t push_many_ops(enum Lexicon *ops, struct Token *origin, struct ExprParserState *state)
{
  struct Expr ex; 
  struct Token tmp, *heap;
  tmp.start = origin->start;
  tmp.end = origin->end;

  for (uint16_t i = 0 ;; i++)
  {
    assert(state->operators_ctr < state->operator_stack_sz);
    if(ops[i] == 0)
      break;

    tmp.type = ops[i];
    heap = vec_push(&state->pool, &tmp);
    assert(heap);

    state->operator_stack[state->operators_ctr] = heap;
    state->operators_ctr += 1;
  }

  return 0;
}

int8_t is_short_blockable(enum Lexicon tok)
{
  enum Lexicon buf[] = {IfBody, RETURN, ELSE, DefBody, 0};
  return contains_tok(tok, buf);
}

bool is_unit_expr(enum Lexicon tok)
{
  return \
    tok == STRING_LITERAL 
    || tok == INTEGER
    || tok == WORD;
}

/*
    precendense table:
      ") ] }"   : 127 non-assoc
      "."     : 126 L
      "^"       : 6 R (1 ^ 2 ^ 3) -> (1 ^ (2 ^ 3))
      "/ * %"   : 5 L  (4 / 2 * 2) -> ((4 / 2) * 2)
      "+ -"     : 4 L
      "!"       : 2 R
      ">> << | &"
      "!= == >= > <= < && ||": 2 L
      "+= -= =" : 1 R
      "( [ { IF ELSE RETURN DEF" : 0 non-assoc
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
    
    else if (is_open_brace(token) || is_op_keyword(token))
        return 0;
    
    return -1;
}