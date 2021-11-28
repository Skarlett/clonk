#include "lexer.h"
#include "../../prelude.h"
#include "../error.h"
#include "debug.h"
#include "helpers.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum Lexicon tokenize_char(char c) {
  uint8_t i = 0;

  switch (c) {
  case ' ':
    return WHITESPACE;
  case '\n':
    return NEWLINE;
  case '\t':
    return WHITESPACE;
  case '\r':
    return WHITESPACE;
  case '/':
    return DIV;
  case '=':
    return EQUAL;
  case '"':
    return QUOTE;
  case '{':
    return BRACE_OPEN;
  case '}':
    return BRACE_CLOSE;
  case '(':
    return PARAM_OPEN;
  case ')':
    return PARAM_CLOSE;
  case '[':
    return BRACKET_OPEN;
  case ']':
    return BRACKET_CLOSE;
  case ';':
    return SEMICOLON;
  case ',':
    return COMMA;
  case '+':
    return ADD;
  case '-':
    return SUB;
  case '*':
    return MUL;
  case '^':
    return POW;
  case '>':
    return GT;
  case '<':
    return LT;
  case '&':
    return AMPER;
  case '|':
    return PIPE;
  case '%':
    return MOD;
  case '_':
    return UNDERSCORE;
  case ':':
    return COLON;
  case '!':
    return NOT;
  case '#':
    return POUND;
  case '@':
    return ATSYM;
  case '.':
    return DOT;
  case '~':
    return TILDE;
  default:
    break;
  }

  for (i = 0; strlen(DIGITS) > i; i++) {
    if (c == DIGITS[i])
      return DIGIT;
  }

  for (i = 0; strlen(ALPHABET) > i; i++) {
    if (c == ALPHABET[i])
      return CHAR;
  }

  return TOKEN_UNDEFINED;
}

int8_t is_compound_bin_op(enum Lexicon tok) {
  return 
    tok == ISEQL 
    || tok == ISNEQL
    || tok == GTEQ
    || tok == LTEQ
    || tok == AND
    || tok == OR
    || tok == MINUSEQ
    || tok == PLUSEQ
    || tok == PIPEOP
    || tok == SHR
    || tok == SHL
    || tok == BOREQL
    || tok == BANDEQL
    || tok == BNEQL;
}

int8_t can_upgrade_token(enum Lexicon token) {
  return token == DIGIT
    || token == CHAR
    || token == UNDERSCORE
    || token == QUOTE
    || token == EQUAL
    || token == NOT
    || token == GT
    || token == LT
    || token == ADD
    || token == SUB
    || token == AMPER
    || token == PIPE
    || token == POUND
    || token == TILDE;
}

int8_t can_ignore_token(enum Lexicon lexed) {
  return lexed == WHITESPACE || lexed == NEWLINE || lexed == COMMENT;
}

int8_t set_compound_token(enum Lexicon *compound_token, enum Lexicon token) {
  switch (token) {
    case DIGIT:
      *compound_token = INTEGER;
      break;

    case NOT:
      *compound_token = ISNEQL;
      break;

    case CHAR:
      *compound_token = WORD;
      break;

    case UNDERSCORE:
      *compound_token = WORD;
      break;

    case QUOTE:
      *compound_token = STRING_LITERAL;
      break;

    case EQUAL:
      *compound_token = ISEQL;
      break;

    case ADD:
      *compound_token = PLUSEQ;
      break;
    
    case GT:
      *compound_token = _COMPOUND_GT;
      break;

    case LT:
      *compound_token = _COMPOUND_LT;
      break;

    case SUB:
      *compound_token = _COMPOUND_SUB;
      break;

    case AMPER:
      *compound_token = _COMPOUND_AMPER;
      break;

    case PIPE:
      *compound_token = _COMPOUND_PIPE;
      break;
    
    case POUND:
      *compound_token = COMMENT;
      break;

    case TILDE:
      *compound_token = BNEQL;
      break;
    
    default:
      return -1;
  }

  return 0;
}

uint16_t token_len(struct Token *tok) { return 1 + tok->end - tok->start; }

/*
    Returns true if `compound_token`
    should continue consuming tokens
*/
int8_t continue_compound_token(
  enum Lexicon token,
  enum Lexicon compound_token,
  uint16_t span_size
){
  return (
      // # ... \n
      compound_token == COMMENT && token != NEWLINE
      || compound_token == INTEGER && token == DIGIT
      
      // ints
      || compound_token == INTEGER && token == UNDERSCORE
      
      // Variables
      || compound_token == WORD && token == CHAR
      || compound_token == WORD && token == UNDERSCORE
      || compound_token == WORD && token == DIGIT
      
      // String literal "..."
      || compound_token == STRING_LITERAL && token != QUOTE
      // !=
      || compound_token == ISNEQL && token == EQUAL && 1 > span_size
      // ==
      || compound_token == ISEQL && token == EQUAL && 1 > span_size
      // +=
      || compound_token == PLUSEQ && token == EQUAL && 1 > span_size
      // ~=
      || compound_token == BNEQL && token == EQUAL && 1 > span_size
      //  `-=` or `-123`
      || compound_token == _COMPOUND_SUB && (token == DIGIT || token == EQUAL && 1 > span_size)
      // `|=`, `|>`, `||`
      || (compound_token == _COMPOUND_PIPE && (token == EQUAL || token == GT || token == PIPE) && 1 > span_size)
      // `&=` `&&`
      || (compound_token == _COMPOUND_AMPER && (token == AMPER || token == EQUAL) && 1 > span_size)
      // `>=` `>>`
      || (compound_token == _COMPOUND_GT && (token == GT || token == EQUAL) && 1 > span_size)
      // `<<` `<=`
      || (compound_token == _COMPOUND_LT && (token == LT || token == EQUAL) && 1 > span_size)
  );    
}

/* used for downgrading compound tokens */
enum Lexicon invert_operator_token(enum Lexicon compound_token) {
  switch (compound_token) {
  case COMMENT:
    return POUND;
  
  case BNEQL:
    return TILDE;
  
  case _COMPOUND_GT:
    return GT;
  case SHR:
    return GT;
  case GTEQ:
    return GT;
  
  case _COMPOUND_LT:
    return LT;
  case SHL:
    return LT;
  case LTEQ:
    return LT;
  
  case _COMPOUND_AMPER:
    return AMPER;
  case AND:
    return AMPER;
  case BANDEQL:
    return AMPER;
  
  case _COMPOUND_PIPE:
    return PIPE;
  case PIPEOP:
    return PIPE;
  case BOREQL:
    return PIPE;
  case OR:
    return PIPE;
  
  case _COMPOUND_SUB:
    return SUB;
  
  case PLUSEQ:
    return ADD;
  
  case ISEQL:
    return EQUAL;
  case ISNEQL:
    return NOT;
  default:
    return TOKEN_UNDEFINED;
  }
}

int8_t derive_keyword(const char *line, struct Token *t) {
  static enum Lexicon lexicon[] = {
    STATIC, CONST,    RETURN, EXTERN, AS,  IF,
    ELSE,   FUNC_DEF, IMPORT, IMPL,   AND, OR
  };

  static char *keywords[] = {
    "static", "const", "return", "extern", "as",
     "if",     "else",  "def",    "import", "impl",
     "and",    "or",    0
  };

  for (uint8_t i = 0; 12 > i; i++) {
    /*token.end+1 since the fields naturally are indexable*/
    if (strlen(keywords[i]) == ((t->end + 1) - t->start) &&
        strncmp(line + t->start, keywords[i], t->end - t->start) == 0) {
      t->type = lexicon[i];
      return 1;
    }
  }

  return 0;
}

int8_t finalize_compound_token(
  struct Token *token, const char *line,
  enum Lexicon lexed,
  struct CompileTimeError *err)
{
  if (token->type == STRING_LITERAL) {
    /*
      Error: string is missing a ending quote
    */
    if (lexed != QUOTE) {
      err->msg = "String missing quote.";
      err->type = Error;
      err->base = token;
      return -1;
    }

    token->end += 1;
  }

  else if (token->type == WORD)
    derive_keyword(line, token);

  /*
  / check if its an operator, and that its lenth is 2
  / if not - down grade the operator from its complex version
  */
  if (2 > token_len(token) && is_compound_bin_op(token->type)) {
    token->type = invert_operator_token(token->type);

    /* Error: UNDEFINED/null token when inverted*/
    if (token->type == TOKEN_UNDEFINED) {
      err->msg = "Unknown operator.";
      err->type = Error;
      err->base = token;
      return -1;
    }
  }

  return 0;
}

int8_t compose_compound(enum Lexicon ctok, enum Lexicon current) {
  if (ctok == _COMPOUND_SUB)
  {
    if (current == DIGIT)
      return INTEGER;
        
    else if (current == EQUAL)
      return MINUSEQ;
  }

  else if (ctok == _COMPOUND_PIPE)
  {
    if (current == PIPE)
      return OR;

    else if (current == GT)
      return PIPEOP;

    else if (current == EQUAL)
      return BOREQL;
  }

  else if (ctok == _COMPOUND_AMPER)
  {
    if (current == AMPER)
      return AND;

    else if (current == EQUAL)
      return BANDEQL;
  }

  else if (ctok == _COMPOUND_LT)
  {
    if(current == LT)
      return SHL;

    else if (current == EQUAL)
      return LTEQ;
  }

  else if (ctok == _COMPOUND_GT)
  {
    if(current == GT)
      return SHR;

    else if (current == EQUAL)
      return GTEQ;
  }

  return 0;
}

int8_t tokenize(
  const char *line,
  struct Token tokens[], uint16_t *token_ctr,
  uint16_t token_sz, struct CompileTimeError *error
){
  struct Token token;
  enum Lexicon current = TOKEN_UNDEFINED, compound_token = TOKEN_UNDEFINED;
  size_t line_len = strlen(line);
  uint16_t start_at = 0;
  uint16_t span_size = 0;
  uint16_t new_tokens = 0;
  bool repeating = false;

  for (uint16_t i = 0; line_len > i; i++) {
    if (line[i] == 0)
      continue;
      
    else if (is_utf(line[i]))
      return -1;
    
    else if (new_tokens > token_sz)
      return -1;

    current = tokenize_char(line[i]);

    if (compound_token == TOKEN_UNDEFINED && can_upgrade_token(current)) {
      set_compound_token(&compound_token, current);
      start_at = i;
      span_size = 0;
      continue;
    }

    /* continuation of complex token */
    else if (continue_compound_token(current, compound_token, span_size)) {
      compound_token = compose_compound(compound_token, current);
      span_size += 1;
      continue;
    }

    else if (compound_token != TOKEN_UNDEFINED) {
      /* completion of complex token */
      repeating = false;
      if (compound_token == COMMENT) {
        compound_token = TOKEN_UNDEFINED;
        current = NEWLINE;
        continue;
      }

      token.start = start_at;
      token.end = start_at + span_size;
      token.type = compound_token;

      if (finalize_compound_token(&token, line, current, error) == -1)
        return -1;

      tokens[new_tokens] = token;
      new_tokens += 1;
      compound_token = TOKEN_UNDEFINED;

      if (token.type == STRING_LITERAL) {
        token.end += 1;
        continue;
      }
    }

    /*
        if the token can be upgraded,
        restart the loop.
    */
    if (can_upgrade_token(current)) {
      i--;
      continue;
    }
    /*
        this fires for non-compound tokens,
        this 'if statement' isn't inside an 'else-block',
        because if a compound token is compleed,
        the current token being lexed still needs to be added.
    */
    else if (current != WHITESPACE && current != NEWLINE &&
             current != TOKEN_UNDEFINED) {
      compound_token = TOKEN_UNDEFINED;

      struct Token token = {.start = i, .end = i, .type = current};

      start_at = 0;
      tokens[new_tokens] = token;
      new_tokens += 1;
    }
  }

  /*
      the source code (char *line) sometimes
      doesn't run into a condition-branch
      where it breaks a complex token's
      continuation before the loop ends

      so forth, if we we're minting a compound token
      it was not stored.

      This checks, and fixes it.
  */
  if (compound_token != TOKEN_UNDEFINED && compound_token != COMMENT) {
    struct Token token = {
        .start = start_at, .end = start_at + span_size, .type = compound_token};

    if (finalize_compound_token(&token, line, current, error) == -1)
      return -1;

    tokens[new_tokens] = token;
    new_tokens += 1;
  }

  // add to counter
  *token_ctr += new_tokens;
  return 0;
}
