#include "expect.h" 
#include "expr.h"
#include "../lexer/lexer.h"
#include "../lexer/helpers.h"

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
  return EXPECTING_CLOSE_BRACKET
    | EXPECTING_CLOSE_PARAM
    | EXPECTING_CLOSE_BRACE;
}

FLAG_T expect_any_open_brace(){
  return EXPECTING_OPEN_BRACKET
    | EXPECTING_OPEN_PARAM
    | EXPECTING_OPEN_BRACE;
}

FLAG_T expect_any_data(){
  return EXPECTING_SYMBOL
    | EXPECTING_INTEGER
    | EXPECTING_STRING;
}

FLAG_T expect_any_op(){
  return EXPECTING_ARITHMETIC_OP
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

FLAG_T expecting_next(enum Lexicon tok)
{
  FLAG_T ret = FLAG_ERROR;

  if (is_symbolic_data(tok))
  {
    set_flag(&ret, EXPECTING_ARITHMETIC_OP      
      | expect_any_close_brace()
      | EXPECTING_DELIMITER);

    if (tok == WORD)
      set_flag(&ret, EXPECTING_OPEN_BRACKET
        | EXPECTING_OPEN_PARAM
        | EXPECTING_APPLY_OPERATOR);
    
    else if(tok == STRING_LITERAL)
      set_flag(&ret, EXPECTING_OPEN_BRACKET 
        | EXPECTING_APPLY_OPERATOR
      );
  }
  
  else if (tok == DOT)
    //(a.b).
    set_flag(&ret, EXPECTING_SYMBOL
      | EXPECTING_NEXT);

  else if (is_operator(tok)) {
    set_flag(&ret, expect_any_data()
      | expect_any_open_brace()
      | EXPECTING_NEXT
    );
    
    if (tok != DOT)
      set_flag(&ret, EXPECTING_INTEGER
        | EXPECTING_STRING);
  }

  else if (tok == COMMA || tok == COLON)
    set_flag(&ret, expect_any_data()
      | expect_any_open_brace()
      | EXPECTING_NEXT
    );
  
  else if (is_open_brace(tok)) {
      set_flag(&ret, expect_any_open_brace()
        | expect_any_data()
        | expect_opposite_brace(tok)
        | EXPECTING_NEXT
      );
    }

  else if (is_close_brace(tok))
     set_flag(&ret, expect_any_close_brace()
        | EXPECTING_OPEN_BRACKET
        | EXPECTING_OPEN_PARAM
        | EXPECTING_DELIMITER
        | expect_any_op()
    );
  
  return ret;
}

enum Lexicon get_expected_delimiter(struct Group *ghead) {
  /* setup delimiter expectation */
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

int8_t is_token_unexpected(struct Token *current, struct Group *ghead, FLAG_T expecting)
{
  enum Lexicon delim;
  FLAG_T flag = 0;

  if (current->type == WORD || current->type == NULLTOKEN)
    flag = EXPECTING_SYMBOL;

  else if (current->type == DOT)
    flag = EXPECTING_APPLY_OPERATOR;

  else if (is_operator(current->type))
    flag = EXPECTING_ARITHMETIC_OP;
  
  else if (is_delimiter(current->type)) {
    delim = get_expected_delimiter(ghead);
    
    if (delim == current->type)
      flag = EXPECTING_DELIMITER;
  }

  else
  {  
    switch (current->type) {
      case INTEGER:
        flag = EXPECTING_INTEGER;
        break;

      case STRING_LITERAL:
        flag = EXPECTING_STRING;
        break;

      case PARAM_OPEN:
        flag = EXPECTING_OPEN_PARAM;
        break;

      case PARAM_CLOSE:
        flag = EXPECTING_CLOSE_PARAM;
        break;

      case BRACKET_OPEN:
        flag = EXPECTING_OPEN_BRACKET;
        break;

      case BRACKET_CLOSE:
        flag = EXPECTING_CLOSE_BRACKET;
        break;

      default: return -1;
    }
  }

  return !check_flag(expecting, flag);
}