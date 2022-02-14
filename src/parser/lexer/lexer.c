#include "lexer.h"
//#include "../../error.h"
#include "../../utils/vec.h"
#include "../../utils/queue.h"

//#include "debug.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#define PREV_BUF_SZ 8
#define ONK_BUF_SZ 65355

struct LexerStage {
    const char * src_code;
    uint16_t src_code_sz;

    enum Lexicon previous_buf[PREV_BUF_SZ];

    /* struct OpenQueue<Lexicon> */
    struct OpenQueue8_t previous;

    enum Lexicon current;
    enum Lexicon compound;

    const uint16_t *i;
    uint16_t cmpd_span_size;
    uint16_t cmpd_start_at;

    /* struct Vec<Token> *ptr */
    struct Vec *tokens;

    /* struct Vec<LexerError> */
    struct Vec errors;

    /* used to construct FROM_LOCATION
     * hijacks the compound token until
     * it falls out of pattern
    */
  enum Lexicon forcing_next_token;
  uint16_t force_start;
  uint16_t force_span;
};

void init_lexer_stage(
  struct LexerStage * stage,
  struct Vec * tokens,
  const char * src_code,
  const uint16_t * i
){
  stage->tokens = tokens;
  stage->i = i;
  stage->src_code = src_code;

  init_queue8(
    &stage->previous,
    stage->previous_buf,
    sizeof(struct Token),
    PREV_BUF_SZ
  );

  /* stage->_is_repeating = false; */
  stage->current = TOKEN_UNDEFINED;
  stage->compound = TOKEN_UNDEFINED;
  stage->forcing_next_token = TOKEN_UNDEFINED;
  stage->cmpd_span_size = 0;
  stage->cmpd_start_at = 0;
  stage->src_code_sz = 0;
};

void init_lexer_output(
  const struct LexerStage *state,
  struct LexerOutput *out
){
  /* avoid const warnings with (void *) */
  assert(memcpy((void *)&out->tokens, &state->tokens, sizeof(struct Vec)) != 0);
  assert(memcpy((void *)&out->errors, &state->errors, sizeof(struct Vec)) != 0);
  out->src_code = state->src_code;
  out->src_code_sz = state->src_code_sz;
}

enum Lexicon tokenize_char(char c) {
  uint8_t i = 0;

  switch (c) {
  case ' ':
    return WHITESPACE;
  case '\n':
  case '\t':
    return WHITESPACE;
  case '\r':
    return WHITESPACE;
  case '/':
    return DIV;
  case '=':
    return EQUAL;
  case '"':
    return D_QUOTE;
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

  case '\\':
    return BACKSLASH;
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
  return tok > __MARKER_COMPOUND_BIN_START
    && __MARKER_COMPOUND_BIN_END > tok;
}

int8_t can_upgrade_token(enum Lexicon token) {
  return (token > __MARKER_UPGRADE_DATA_START && __MARKER_UPGRADE_DATA_END)
    || (token > __MARKER_UPGRADE_OP_START && __MARKER_UPGRADE_OP_END > token)
    || (token > __MARKER_UNARY_START && __MARKER_UNARY_END > token)
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

    case D_QUOTE:
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

uint16_t token_len(struct Token *tok)
{
  return 1 + tok->end - tok->start;
}

/*
    Returns true if `compound_token`
    should continue consuming tokens
*/
int8_t continue_compound_token(
  struct LexerStage *state
){
  struct Token * prev;
  enum Lexicon compound_token = state->compound,
    token = state->current;
  uint16_t span_size = state->cmpd_span_size;

  prev = 0;
  if (state->previous.nitems > 0)
     prev = queue8_head(&state->previous);

  return (
      (compound_token == COMMENT && token != NEWLINE)
      // # ... \n
      || (compound_token == INTEGER && token == DIGIT)
      
      // ints
      || (compound_token == INTEGER && token == UNDERSCORE)
      
      // Variables
      || (compound_token == WORD && token == CHAR)
      || (compound_token == WORD && token == UNDERSCORE)
      || (compound_token == WORD && token == DIGIT)
      
      // String literal "..."
      ||(compound_token == STRING_LITERAL
        && (
          (token != D_QUOTE || token == D_QUOTE && prev && prev == BACKSLASH)
          // TODO:
          //|| (token != S_QUOTE || token == S_QUOTE && prev == BACKSLASH)
          )
        )
      // !=
      || (compound_token == ISNEQL && token == EQUAL && 1 > span_size)
      // ==
      || (compound_token == ISEQL && token == EQUAL && 1 > span_size)
      // +=
      || (compound_token == PLUSEQ && token == EQUAL && 1 > span_size)
      // ~=
      || (compound_token == BNEQL && token == EQUAL && 1 > span_size)
      //  `-=` or `-123`
      || (compound_token == _COMPOUND_SUB && (token == DIGIT || (token == EQUAL && 1 > span_size)))
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

int8_t derive_keyword(const char *src_code, struct Token *t) {
  static enum Lexicon lexicon[] = {
    //STATIC, CONST,
    RETURN,
    FOR, WHILE,
    // EXTERN, AS,
    IN, IF,
    ELSE, FUNC_DEF,
    IMPORT, FROM,
    STRUCT, IMPL,
    AND, OR, 0
  };

  static char *keywords[] = {
    // "static", "const",
    "return", "for", "while",
    "in",
    //"extern", "as",
    "if", "else",  "def",
    "import", "from",
    "struct", "impl",
    "and", "or", 0
  };

  for (uint8_t i = 0 ;; i++) {
    if (lexicon[i] == 0)
      break;
    
    /* token.end + 1 due to index position */
    if (strlen(keywords[i]) == ((t->end + 1) - t->start) &&
        strncmp(src_code + t->start, keywords[i], t->end - t->start) == 0) {
      t->type = lexicon[i];
      return 1;
    }
  }
  return 0;
}


/*
** Because our lexer kind of sucks,
** this is using the current token to determine
** what the compound token should be,
** for example `-` (MINUS) can turn into many
** different compounds `-=`, `-1`.
*/
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

int8_t finalize_compound_token(
  struct LexerStage *state,
  struct Token *token,
  enum Lexicon lexed
){
  struct LexerError err;
  if (token->type == STRING_LITERAL)
  {

    /*
      Error: string is missing a ending quote
    */
    if (lexed != D_QUOTE) {
      err.type = lex_err_missing_end_quote;

      assert(memcpy(
        &err.type_data.bad_str,
        token,
        sizeof(struct Token))
      );

      /* push error */
      vec_push(&state->errors, &err);
      return -1;
    }

    token->end += 1;
  }

  else if (token->type == WORD)
    derive_keyword(state->src_code, token);

  /*
  / check if its an operator, and that its lenth is 2
  / if not - down grade the operator from its complex version
  */
  if (token_len(token) < 2 && is_compound_bin_op(token->type)) {
    token->type = invert_operator_token(token->type);

    /* Error: UNDEFINED/null token when inverted*/
    assert(token->type != TOKEN_UNDEFINED);
  }

  return 0;
}

int8_t push_tok(
  struct LexerStage *state,
  struct Token *tok)
{
  enum Lexicon type = state->current;
  int8_t ret = 0;

  if (state->compound != TOKEN_UNDEFINED)
    type = state->compound;

  ret = finalize_compound_token(
    state,
    tok,
    type
  );

  assert(vec_push(state->tokens, tok) != 0);
  queue8_push(&state->previous, &tok);


  if (type == FROM)
  {
    assert(UINT16_MAX > *state->i);

    state->forcing_next_token = FROM_LOCATION;
    state->force_start = *state->i + 1;
  }
  return ret;
}


int8_t continue_forcing(struct LexerStage *state)
{
  return state->current == WORD || state->current == DOT;
}

/******
 * ----
 * TODO: escape quotes `\"` and `\'`
 * TODO: single quote strings `'hello world'`
 * TODO: triple quoted string that
 *       interprets carriage return literallty
 *       `'''this lets new src_codes'''`
 * TODO: whitespace interpretation `\n\t\r`
 * TODO: negative numbers
 * ----
 ******/
int8_t tokenize(
  struct LexerInput *in,
  struct LexerOutput *out
){
  uint16_t i = 0;
  uint16_t utf_error_flag = 0;

  struct LexerStage state;
  struct Token token;
  struct LexerError err;

  init_lexer_stage(
    &state,
    &in->tokens,
    in->src_code,
    &i
  );

  for (i = 0 ;; i++) {
    memset(&token, 0, sizeof(struct Token));

    if (state.src_code[i] == 0)
      break;

    /* Input size check */
    if (ONK_BUF_SZ > state.src_code_sz)
      state.src_code_sz += 1;

    else {
      err.type = lex_err_input_too_big;
      assert(vec_push(&state.errors, &err) != 0);
      return -1;
    }

    /* Capture & throw error on non-ascii character streams */
    if (is_utf(state.src_code[i])) {
      utf_error_flag = i;
      continue;
    }

    else if(utf_error_flag > 0)
    {
      if (state.src_code[i] == ' '
          || state.src_code[i] == '\t'
          || state.src_code[i] == '\n')
      {
        token.start = utf_error_flag;
        token.end = (i || 1) - 1;
        token.type = TOKEN_UNDEFINED;

        err.type = lex_err_non_ascii_token;
        err.type_data.non_ascii_token = token;

        /* push error */
        vec_push(&state.errors, &err);

        utf_error_flag = 0;
      }

      continue;
    }

    state.current = tokenize_char(state.src_code[i]);

    /* create compound token*/
    if (state.compound == TOKEN_UNDEFINED && can_upgrade_token(state.current)) {
      set_compound_token(&state.compound, state.current);
      state.cmpd_start_at = i;
      state.cmpd_span_size = 0;
      continue;
    }
    else if (state.forcing_next_token != TOKEN_UNDEFINED && continue_forcing(&state))
    {
      state.force_span += 1;
      continue;
    }

    /* continuation of complex token */
    else if (continue_compound_token(&state)) {
      state.compound = compose_compound(state.compound, state.current);
      state.cmpd_span_size += 1;
      queue8_push(&state.previous, &token);
      continue;
    }

    /* completed compound token */
    else if (state.compound != TOKEN_UNDEFINED || state.forcing_next_token) {
      // state._is_repeating = false;

      if (state.compound == COMMENT)
      {
        state.compound = TOKEN_UNDEFINED;
        continue;
      }

      if(state.compound != TOKEN_UNDEFINED)
      {
        token.start = state.cmpd_start_at;
        token.end = state.cmpd_start_at + state.cmpd_span_size;
        token.type = state.compound;
        token.seq = state.tokens->len;
        state.compound = TOKEN_UNDEFINED;
      }
      else {
        token.start = state.force_start;
        token.end = state.force_start + state.force_span;
        token.type = state.forcing_next_token;
        token.seq = state.tokens->len;

        state.forcing_next_token = TOKEN_UNDEFINED;
        state.force_start = 0;
        state.force_span = 0;
      }

      push_tok(&state, &token);


      if (token.type == STRING_LITERAL)
      {
        token.end += 1;
        continue;
      }
    }

    /*
        if the token can be upgraded, step the loop back by 1.
        This allows the token to fall through the entire loop.
        So that its turned into a compound token.
    */
    if (can_upgrade_token(state.current)) {
      i--;
      continue;
    }

    /*
        this fires for non-compound tokens,
        this 'if statement' isn't inside an 'else-block',
        because if a compound token is compleed,
        the current token being lexed still needs to be added.
    */
    else if (state.current != WHITESPACE
             && state.current != TOKEN_UNDEFINED)
    {
      state.compound = TOKEN_UNDEFINED;

      token.start = i;
      token.end = i;
      token.type = state.current;
      token.seq = state.tokens->len;

      vec_push(state.tokens, &token);
      state.cmpd_start_at = 0;
    }
  }

  /*
      the source code (char *src_code) sometimes
      doesn't run into a condition-branch
      where it breaks a complex token's
      continuation before the loop ends

      so forth, if we we're minting a compound token
      it was not stored.

      This checks, and fixes it.
  */
  if (state.compound != TOKEN_UNDEFINED && state.compound != COMMENT)
  {
    token.start = state.cmpd_start_at;
    token.end = state.cmpd_start_at + state.cmpd_span_size;
    token.type = state.compound;
    token.seq = state.tokens->len;

    if (push_tok(&state, &token) == -1)
      return -1;

    //assert(vec_push(state.tokens, &token) > 0);
  }
  
  token.type = EOFT;
  token.start = i;
  token.end = i;
  token.seq = state.tokens->len;
  vec_push(state.tokens, &token);

  /* setup return values */
  init_lexer_output(&state, out);
  return 0;
}
