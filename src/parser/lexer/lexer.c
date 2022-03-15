/******
 * ----
 * TODO: escape quotes `\"` and `\'`
 * TODO: single quote strings `'hello world'`
 * TODO: triple quoted string that
 *       interprets carriage return literallty
 *       `'''this lets new src_codes'''`
 * TODO: whitespace interpretation `\n\t\r`
 * TODO: negative numbers (maybe done?)
 *
 * ----
*/

#include <stdio.h>
#include "clonk.h"
#include "lexer.h"
#include "onkstd/queue.h"
#include "onkstd/vec.h"

#define PREV_BUF_SZ 8
#define ONK_BUF_SZ 65355

struct LexerStage {
    const char * src_code;
    uint16_t src_code_sz;

    enum onk_lexicon_t previous_buf[PREV_BUF_SZ];

    /* struct OpenQueue<onk_lexicon_t> */
    struct onk_open_queue_t previous;

    enum onk_lexicon_t current;
    enum onk_lexicon_t compound;

    const uint16_t *i;
    uint16_t cmpd_span_size;
    uint16_t cmpd_start_at;

    /* struct onk_vec_t<onk_token_t> *ptr */
    struct onk_vec_t *tokens;

    /* struct onk_vec_t<onk_lexer_error_t> */
    struct onk_vec_t errors;

    /* used to construct ONK_FROM_LOCATION
     * hijacks the compound token until
     * it falls out of pattern
    */
  enum onk_lexicon_t forcing_next_token;
  uint16_t force_start;
  uint16_t force_span;
};

void init_lexer_stage(
  struct LexerStage * stage,
  struct onk_vec_t * tokens,
  const char * src_code,
  const uint16_t * i
){
  stage->tokens = tokens;
  stage->i = i;
  stage->src_code = src_code;

  onk_init_queue8(
    &stage->previous,
    stage->previous_buf,
    sizeof(struct onk_token_t),
    PREV_BUF_SZ
  );

  /* stage->_is_repeating = false; */
  stage->current = ONK_UNDEFINED_TOKEN;
  stage->compound = ONK_UNDEFINED_TOKEN;
  stage->forcing_next_token = ONK_UNDEFINED_TOKEN;
  stage->cmpd_span_size = 0;
  stage->cmpd_start_at = 0;
  stage->src_code_sz = 0;
};

void init_lexer_output(
  const struct LexerStage *state,
  struct onk_lexer_output_t *out
){
  /* avoid const warnings with (void *) */
  assert(memcpy((void *)&out->tokens, &state->tokens, sizeof(struct onk_vec_t)) != 0);
  assert(memcpy((void *)&out->errors, &state->errors, sizeof(struct onk_vec_t)) != 0);
  out->src_code = state->src_code;
  out->src_code_sz = state->src_code_sz;
}

enum onk_lexicon_t onk_tokenize_char(char c) {
  uint8_t i = 0;

  switch (c) {
  case ' ':
    return ONK_WHITESPACE_TOKEN;
  case '\n':
  case '\t':
    return ONK_WHITESPACE_TOKEN;
  case '\r':
    return ONK_WHITESPACE_TOKEN;
  case '/':
    return ONK_DIV_TOKEN;
  case '=':
    return ONK_EQUAL_TOKEN;
  case '"':
    return ONK_DOUBLE_QUOTE_TOKEN;
  case '{':
    return ONK_BRACE_OPEN_TOKEN;
  case '}':
    return ONK_BRACE_CLOSE_TOKEN;
  case '(':
    return ONK_PARAM_OPEN_TOKEN;
  case ')':
    return ONK_PARAM_CLOSE_TOKEN;
  case '[':
    return ONK_BRACKET_OPEN_TOKEN;
  case ']':
    return ONK_BRACKET_CLOSE_TOKEN;
  case ';':
    return ONK_SEMICOLON_TOKEN;
  case ',':
    return ONK_COMMA_TOKEN;
  case '+':
    return ONK_ADD_TOKEN;
  case '-':
    return ONK_SUB_TOKEN;
  case '*':
    return ONK_MUL_TOKEN;
  case '^':
    return ONK_POW_TOKEN;
  case '>':
    return ONK_GT_TOKEN;
  case '<':
    return ONK_LT_TOKEN;
  case '&':
    return ONK_AMPER_TOKEN;
  case '|':
    return ONK_PIPE_TOKEN;
  case '%':
    return ONK_MOD_TOKEN;
  case '_':
    return ONK_UNDERSCORE_TOKEN;
  case ':':
    return ONK_COLON_TOKEN;
  case '!':
    return ONK_NOT_TOKEN;
  case '#':
    return POUND;
  /* case '@': */
  /*   return ONK_ATSYM__TOKEN; */
  case '.':
    return ONK_DOT_TOKEN;
  case '~':
    return ONK_TILDE_TOKEN;
  case '$':
    return ONK_DOLLAR_TOKEN;

  case '\\':
    return ONK_BACKSLASH_TOKEN;
  default:
    break;
  }

  /* £ -> $ */
  if ((unsigned char)c == 0xa3)
    return ONK_DOLLAR_TOKEN;

  for (i = 0; strlen(ONK_DIGIT_STR) > i; i++) {
    if (c == ONK_DIGIT_STR[i])
      return ONK_DIGIT_TOKEN;
  }

  for (i = 0; strlen(ONK_ALPHABET) > i; i++) {
    if (c == ONK_ALPHABET[i])
      return ONK_CHAR_TOKEN;
  }

  return ONK_UNDEFINED_TOKEN;
}

int8_t is_compound_bin_op(enum onk_lexicon_t tok) {
  return tok > __ONK_MARKER_COMPOUND_BIN_START
    && __ONK_MARKER_COMPOUND_BIN_END > tok;
}

int8_t can_upgrade_token(enum onk_lexicon_t token) {
  return (token > __ONK_MARKER_UPGRADE_DATA_START && __ONK_MARKER_UPGRADE_DATA_END > token)
    || (token > __ONK_MARKER_UPGRADE_OP_START && __ONK_MARKER_UPGRADE_OP_END > token)
    || (token > __ONK_MARKER_UNARY_START && __ONK_MARKER_UNARY_END > token);
}

int8_t set_compound_token(enum onk_lexicon_t *compound_token, enum onk_lexicon_t token) {
  switch (token) {
    case ONK_DIGIT_TOKEN:
      *compound_token = ONK_INTEGER_TOKEN;
      break;

    case ONK_NOT_TOKEN:
      *compound_token = ONK_NOT_EQL_TOKEN;
      break;

    case ONK_CHAR_TOKEN:
      *compound_token = ONK_WORD_TOKEN;
      break;

    case ONK_UNDERSCORE_TOKEN:
      *compound_token = ONK_WORD_TOKEN;
      break;

    case ONK_DOUBLE_QUOTE_TOKEN:
      *compound_token = ONK_STRING_LITERAL_TOKEN;
      break;

    case ONK_EQUAL_TOKEN:
      *compound_token = ONK_ISEQL_TOKEN;
      break;

    case ONK_ADD_TOKEN:
      *compound_token = ONK_PLUSEQ_TOKEN;
      break;
    
    case ONK_GT_TOKEN:
      *compound_token = _ONK_GT_TRANSMISSION_TOKEN;
      break;

    case ONK_LT_TOKEN:
      *compound_token = _ONK_LT_TRANSMISSION_TOKEN;
      break;

    case ONK_SUB_TOKEN:
      *compound_token = _ONK_SUB_TRANSMISSION_TOKEN;
      break;

    case ONK_AMPER_TOKEN:
      *compound_token = _ONK_AMPER_TRANSMISSION_TOKEN;
      break;

    case ONK_PIPE_TOKEN:
      *compound_token = _ONK_PIPE_TRANSMISSION_TOKEN;
      break;
    
    case POUND:
      *compound_token = ONK_COMMENT_TOKEN;
      break;

    case ONK_TILDE_TOKEN:
      *compound_token = ONK_BIT_NOT_EQL;
      break;

    case ONK_DOLLAR_TOKEN:
      *compound_token = _ONK_DOLLAR_TRANSMISSION_TOKEN;
      break;

    default:
      return -1;
  }

  return 0;
}

uint16_t token_len(struct onk_token_t *tok)
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
  struct onk_token_t * prev;
  enum onk_lexicon_t compound_token = state->compound,
    token = state->current;
  uint16_t span_size = state->cmpd_span_size;

  prev = 0;
  if (state->previous.nitems > 0)
     prev = onk_queue8_head(&state->previous);

  return (
      (compound_token == ONK_COMMENT_TOKEN && token != ONK_NEWLINE_TOKEN)
      // # ... \n
      || (compound_token == ONK_INTEGER_TOKEN && token == ONK_DIGIT_TOKEN)
      
      // ints
      || (compound_token == ONK_INTEGER_TOKEN && token == ONK_UNDERSCORE_TOKEN)
      
      // Variables
      || (compound_token == ONK_WORD_TOKEN && token == ONK_CHAR_TOKEN)
      || (compound_token == ONK_WORD_TOKEN && token == ONK_UNDERSCORE_TOKEN)
      || (compound_token == ONK_WORD_TOKEN && token == ONK_DIGIT_TOKEN)
      
      // String literal "..."
      ||(compound_token == ONK_STRING_LITERAL_TOKEN
        && (
          (token != ONK_DOUBLE_QUOTE_TOKEN || (token == ONK_DOUBLE_QUOTE_TOKEN && prev && prev->type == ONK_BACKSLASH_TOKEN))
          // TODO:
          //|| (token != ONK_SINGLE_QUOTE_TOKEN || token == ONK_SINGLE_QUOTE_TOKEN && prev == ONK_BACKSLASH_TOKEN)
          )
        )
      // !=
      || (compound_token == ONK_NOT_EQL_TOKEN && token == ONK_EQUAL_TOKEN && 1 > span_size)
      // ==
      || (compound_token == ONK_ISEQL_TOKEN && token == ONK_EQUAL_TOKEN && 1 > span_size)
      // +=
      || (compound_token == ONK_PLUSEQ_TOKEN && token == ONK_EQUAL_TOKEN && 1 > span_size)
      // ~=
      || (compound_token == ONK_BIT_NOT_EQL && token == ONK_EQUAL_TOKEN && 1 > span_size)
      //  `-=` or `-123`
      || (compound_token == _ONK_SUB_TRANSMISSION_TOKEN && (token == ONK_DIGIT_TOKEN || (token == ONK_EQUAL_TOKEN && 1 > span_size)))
      // `|=`, `|>`, `||`
      || (compound_token == _ONK_PIPE_TRANSMISSION_TOKEN && (token == ONK_EQUAL_TOKEN || token == ONK_GT_TOKEN || token == ONK_PIPE_TOKEN) && 1 > span_size)
      // `&=` `&&`
      || (compound_token == _ONK_AMPER_TRANSMISSION_TOKEN && (token == ONK_AMPER_TOKEN || token == ONK_EQUAL_TOKEN) && 1 > span_size)
      // `>=` `>>`
      || (compound_token == _ONK_GT_TRANSMISSION_TOKEN && (token == ONK_GT_TOKEN || token == ONK_EQUAL_TOKEN) && 1 > span_size)
      // `<<` `<=`
      || (compound_token == _ONK_LT_TRANSMISSION_TOKEN && (token == ONK_LT_TOKEN || token == ONK_EQUAL_TOKEN) && 1 > span_size)
      // ${
      || (compound_token == _ONK_DOLLAR_TRANSMISSION_TOKEN && token == ONK_BRACE_OPEN_TOKEN && 1 > span_size)
  );
}


bool downgrade_cmpd_gt(enum onk_lexicon_t tok)
{
  return tok == _ONK_GT_TRANSMISSION_TOKEN
    || tok == ONK_SHR_TOKEN
    || tok == ONK_GT_EQL_TOKEN;
}

bool downgrade_cmpd_lt(enum onk_lexicon_t tok)
{
  return tok == _ONK_LT_TRANSMISSION_TOKEN
    || tok == ONK_SHL_TOKEN
    || tok == ONK_LT_EQL_TOKEN;
}

/* used for downgrading compound tokens */
enum onk_lexicon_t invert_operator_token(enum onk_lexicon_t compound_token) {

  if(downgrade_cmpd_gt(compound_token))
    return ONK_GT_TOKEN;

  else if(downgrade_cmpd_lt(compound_token))
    return ONK_LT_TOKEN;

  else switch (compound_token) {

  case ONK_COMMENT_TOKEN:
    return POUND;
  
  case ONK_BIT_NOT_EQL:
    return ONK_TILDE_TOKEN;

  case _ONK_AMPER_TRANSMISSION_TOKEN:
    return ONK_AMPER_TOKEN;

  case ONK_AND_TOKEN:
    return ONK_AMPER_TOKEN;
  case ONK_BIT_AND_EQL:
    return ONK_AMPER_TOKEN;

  case _ONK_PIPE_TRANSMISSION_TOKEN:
    return ONK_PIPE_TOKEN;
  case ONK_BIT_OR_EQL:
    return ONK_PIPE_TOKEN;
  case ONK_OR_TOKEN:
    return ONK_PIPE_TOKEN;
  
  case _ONK_SUB_TRANSMISSION_TOKEN:
    return ONK_SUB_TOKEN;

  case _ONK_DOLLAR_TRANSMISSION_TOKEN:
    return ONK_DOLLAR_TOKEN;

  case ONK_PLUSEQ_TOKEN:
    return ONK_ADD_TOKEN;
  
  case ONK_ISEQL_TOKEN:
    return ONK_EQUAL_TOKEN;
  case ONK_NOT_EQL_TOKEN:
    return ONK_NOT_TOKEN;

  default:
    return ONK_UNDEFINED_TOKEN;
  }
}

int8_t derive_keyword(const char *src_code, struct onk_token_t *t) {
  static enum onk_lexicon_t lexicon[] = {
    //STATIC, CONST,
    ONK_RETURN_TOKEN,
    ONK_FOR_TOKEN, ONK_WHILE_TOKEN,
    // EXTERN, AS,
    ONK_TRUE_TOKEN, ONK_FALSE_TOKEN,
    ONK_IN_TOKEN, ONK_IF_TOKEN,
    ONK_ELSE_TOKEN, ONK_DEF_TOKEN,
    ONK_IMPORT_TOKEN, ONK_FROM_TOKEN,
    ONK_STRUCT_TOKEN, ONK_IMPL_TOKEN,
    ONK_AND_TOKEN, ONK_OR_TOKEN, 0
  };

  static char *keywords[] = {
    // "static", "const",
    "return", "for", "while",
    "true", "false",
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
enum onk_lexicon_t compose_compound(enum onk_lexicon_t ctok, enum onk_lexicon_t current) {
  if (ctok == _ONK_SUB_TRANSMISSION_TOKEN)
  {
    if (current == ONK_DIGIT_TOKEN)
      return ONK_INTEGER_TOKEN;
        
    else if (current == ONK_EQUAL_TOKEN)
      return ONK_MINUS_EQL_TOKEN;
  }

  else if (ctok == _ONK_PIPE_TRANSMISSION_TOKEN){
    if (current == ONK_PIPE_TOKEN)
      return ONK_OR_TOKEN;

    else if (current == ONK_EQUAL_TOKEN)
      return ONK_BIT_OR_EQL;
  }

  else if (ctok == _ONK_AMPER_TRANSMISSION_TOKEN)
  {
    if (current == ONK_AMPER_TOKEN)
      return ONK_AND_TOKEN;

    else if (current == ONK_EQUAL_TOKEN)
      return ONK_BIT_AND_EQL;
  }

  else if (ctok == _ONK_LT_TRANSMISSION_TOKEN)
  {
    if(current == ONK_LT_TOKEN)
      return ONK_SHL_TOKEN;

    else if (current == ONK_EQUAL_TOKEN)
      return ONK_LT_EQL_TOKEN;
  }

  else if (ctok == _ONK_GT_TRANSMISSION_TOKEN)
  {
    if(current == ONK_GT_TOKEN)
      return ONK_SHR_TOKEN;

    else if (current == ONK_EQUAL_TOKEN)
      return ONK_GT_EQL_TOKEN;
  }

  else if (ctok == _ONK_DOLLAR_TRANSMISSION_TOKEN)
    return ONK_HASHMAP_LITERAL_START_TOKEN;

  return 0;
}

int8_t finalize_compound_token(
  struct LexerStage *state,
  struct onk_token_t *token,
  enum onk_lexicon_t lexed
){
  struct onk_lexer_error_t err;
  if (token->type == ONK_STRING_LITERAL_TOKEN)
  {

    /*
      Error: string is missing a ending quote
    */
    if (lexed != ONK_DOUBLE_QUOTE_TOKEN) {
      err.type = lex_err_missing_end_quote;

      assert(memcpy(
        &err.type_data.bad_str,
        token,
        sizeof(struct onk_token_t))
      );

      /* push error */
      onk_vec_push(&state->errors, &err);
      return -1;
    }

    token->end += 1;
  }

  else if (token->type == ONK_WORD_TOKEN)
    derive_keyword(state->src_code, token);

  /*
  / check if its an operator, and that its lenth is 2
  / if not - down grade the operator from its complex version
  */
  if (token_len(token) < 2 && is_compound_bin_op(token->type)) {
    token->type = invert_operator_token(token->type);

    /* Error: UNDEFINED/null token when inverted*/
    assert(token->type != ONK_UNDEFINED_TOKEN);
  }

  return 0;
}

int8_t push_tok(
  struct LexerStage *state,
  struct onk_token_t *tok)
{
  enum onk_lexicon_t type = state->current;
  int8_t ret = 0;

  if (state->compound != ONK_UNDEFINED_TOKEN)
    type = state->compound;

  ret = finalize_compound_token(
    state,
    tok,
    type
  );

  assert(onk_vec_push(state->tokens, tok) != 0);
  onk_queue8_push(&state->previous, &tok);


  if (type == ONK_FROM_TOKEN)
  {
    assert(UINT16_MAX > *state->i);

    state->forcing_next_token = ONK_FROM_LOCATION;
    state->force_start = *state->i + 1;
  }
  return ret;
}


int8_t continue_forcing(struct LexerStage *state)
{
  return state->current == ONK_WORD_TOKEN || state->current == ONK_DOT_TOKEN;
}

int8_t onk_tokenize(
  struct onk_lexer_input_t *in,
  struct onk_lexer_output_t *out
){
  uint16_t i = 0;
  uint16_t utf_error_flag = 0;

  struct LexerStage state;
  struct onk_token_t token;
  struct onk_lexer_error_t err;

  init_lexer_stage(
    &state,
    &in->tokens,
    in->src_code,
    &i
  );

  for (i = 0 ;; i++) {
    memset(&token, 0, sizeof(struct onk_token_t));

    if (state.src_code[i] == 0)
      break;

    /* Input size check */
    if (ONK_BUF_SZ > state.src_code_sz)
      state.src_code_sz += 1;

    else {
      err.type = lex_err_input_too_big;
      assert(onk_vec_push(&state.errors, &err) != 0);
      return -1;
    }

    /* Capture & throw error on non-ascii character streams */
    if (onk_is_utf_byte(state.src_code[i])) {
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
        token.type = ONK_UNDEFINED_TOKEN;

        err.type = lex_err_non_ascii_token;
        err.type_data.non_ascii_token = token;

        /* push error */
        onk_vec_push(&state.errors, &err);

        utf_error_flag = 0;
      }

      continue;
    }

    state.current = onk_tokenize_char(state.src_code[i]);

    /* create compound token*/
    if (state.compound == ONK_UNDEFINED_TOKEN && can_upgrade_token(state.current)) {
      set_compound_token(&state.compound, state.current);
      state.cmpd_start_at = i;
      state.cmpd_span_size = 0;
      continue;
    }
    else if (state.forcing_next_token != ONK_UNDEFINED_TOKEN && continue_forcing(&state))
    {
      state.force_span += 1;
      continue;
    }

    /* continuation of complex token */
    else if (continue_compound_token(&state)) {
      state.compound = compose_compound(state.compound, state.current);
      state.cmpd_span_size += 1;
      onk_queue8_push(&state.previous, &token);
      continue;
    }

    /* completed compound token */
    else if (state.compound != ONK_UNDEFINED_TOKEN || state.forcing_next_token) {
      // state._is_repeating = false;

      /* if (state.compound == ONK_COMMENT_TOKEN) */
      /* { */
      /*   state.compound = ONK_UNDEFINED_TOKEN; */
      /*   continue; */
      /* } */

      if(state.compound != ONK_UNDEFINED_TOKEN)
      {
        token.start = state.cmpd_start_at;
        token.end = state.cmpd_start_at + state.cmpd_span_size;
        token.type = state.compound;
        token.seq = state.tokens->len;
        state.compound = ONK_UNDEFINED_TOKEN;
      }
      else {
        token.start = state.force_start;
        token.end = state.force_start + state.force_span;
        token.type = state.forcing_next_token;
        token.seq = state.tokens->len;

        state.forcing_next_token = ONK_UNDEFINED_TOKEN;
        state.force_start = 0;
        state.force_span = 0;
      }

      push_tok(&state, &token);


      if (token.type == ONK_STRING_LITERAL_TOKEN)
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
      i -= 1;
      continue;
    }

    /*
        this fires for non-compound tokens,
        this 'if statement' isn't inside an 'else-block',
        because if a compound token is compleed,
        the current token being lexed still needs to be added.
    */
    else if (state.current != ONK_UNDEFINED_TOKEN)
    {
      state.compound = ONK_UNDEFINED_TOKEN;

      token.start = i;
      token.end = i;
      token.type = state.current;
      token.seq = state.tokens->len;

      onk_vec_push(state.tokens, &token);
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
  if (state.compound != ONK_UNDEFINED_TOKEN)
  {
    token.start = state.cmpd_start_at;
    token.end = state.cmpd_start_at + state.cmpd_span_size;
    token.type = state.compound;
    token.seq = state.tokens->len;

    if (push_tok(&state, &token) == -1)
      return -1;

    //assert(onk_vec_push(state.tokens, &token) > 0);
  }
  
  token.type = ONK_EOFT;
  token.start = i;
  token.end = i;
  token.seq = state.tokens->len;
  onk_vec_push(state.tokens, &token);

  /* setup return values */
  init_lexer_output(&state, out);
  return 0;
}
