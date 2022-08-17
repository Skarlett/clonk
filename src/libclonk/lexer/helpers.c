#include "clonk.h"
#include "lexer.h"

bool onk_is_tok_delimiter(enum onk_lexicon_t token) {
    return token > PH_ONK_MARKER_DELIM_START
        && PH_ONK_MARKER_DELIM_END > token;
}

bool onk_is_tok_brace(enum onk_lexicon_t token) {
    return token > PH_ONK_MARKER_BRACE_START
        && PH_ONK_MARKER_BRACE_END > token;
}

bool onk_is_tok_close_brace(enum onk_lexicon_t token) {
    return token > PH_ONK_MARKER_CLOSE_BRACE_START
        && PH_ONK_MARKER_CLOSE_BRACE_END > token;
}

bool onk_is_tok_open_brace(enum onk_lexicon_t token) {
    return (token > PH_ONK_MARKER_OPEN_BRACE_START
      && PH_ONK_MARKER_OPEN_BRACE_END > token);
}

bool onk_is_tok_unit(enum onk_lexicon_t token) {
    return token > PH_ONK_MARKER_UNIT_START
        && PH_ONK_MARKER_UNIT_END > token;
}

bool onk_is_tok_asn_operator(enum onk_lexicon_t token) {
    return token > PH_ONK_MARKER_ASN_START
        && PH_ONK_MARKER_ASN_END > token;
}

bool onk_is_tok_binop(enum onk_lexicon_t token) {
    return token > PH_ONK_MARKER_BIN_START
        && PH_ONK_MARKER_BIN_END > token;
}

bool onk_is_tok_unary_operator(enum onk_lexicon_t token) {
    return token > PH_ONK_MARKER_UNARY_START
        && PH_ONK_MARKER_UNARY_END > token;
}

bool onk_is_tok_operator(enum onk_lexicon_t token) {
    return token > PH_ONK_MARKER_OP_START
        && PH_ONK_MARKER_OP_END > token;
}

bool onk_is_tok_block_keyword(enum onk_lexicon_t token) {
    return token > PH_ONK_MARKER_KEYWORD_BLOCK_START
        && PH_ONK_MARKER_KEYWORD_BLOCK_END > token;
}

bool onk_is_tok_data_keyword(enum onk_lexicon_t token){
    return token > PH_ONK_MARKER_KEYWORD_DATA_START
        && PH_ONK_MARKER_KEYWORD_DATA_END > token;
}

bool onk_is_tok_keyword(enum onk_lexicon_t token){
    return onk_is_tok_block_keyword(token)
        || onk_is_tok_data_keyword(token);
}

bool onk_do_default_expectation(enum onk_lexicon_t token){
    return token > PH_ONK_MARKER_KEYWORD_DATA_START
        && PH_ONK_MARKER_KEYWORD_DATA_END > token;
}

bool onk_is_tok_illegal(enum onk_lexicon_t token){
    return token > PH_ONK_MARKER_ILLEGAL_INPUT_START
        && PH_ONK_MARKER_ILLEGAL_INPUT_END > token;
}


/* keywords that can only be used in blocks */
/* bool onk_is_tok_kword_block(enum onk_lexicon_t token) { */
/*     return token > PH_ONK_MARKER_KEYWORD_BLOCK_START */
/*         && PH_ONK_MARKER_KEYWORD_BLOCK_END > token; */
/* } */

bool onk_is_group(enum onk_lexicon_t tok) {
    return tok > PH_ONK_MARKER_GROUP_START
        && PH_ONK_MARKER_GROUP_END > tok;
}

bool onk_is_tok_group_modifier(enum onk_lexicon_t tok) {
    return tok > PH_ONK_MARKER_GROUP_OP_START
        && PH_ONK_MARKER_GROUP_OP_END > tok;
}

bool onk_is_tok_loopctlkw(enum onk_lexicon_t tok) {
    return tok > PH_ONK_MARKER_LOOP_CTL_START
        && PH_ONK_MARKER_LOOP_CTL_END > tok;
}

bool onk_is_tok_whitespace(enum onk_lexicon_t tok) {
    return tok > PH_ONK_MARKER_WHITESPACE_START
        && PH_ONK_MARKER_WHITESPACE_END > tok;
}

bool onk_is_int_tok_negative(const char * source, struct onk_token_t *token) {
    return token->type == ONK_INTEGER_TOKEN
        && *(source + token->start) == '-';
}

bool onk_is_utf_byte(char ch) {
    unsigned char copy = ch;

    return (copy >= 0x80
        /* accept a cup of tea (£) */
        && copy != 0xa3);
}

uint16_t onk_lexarr_contains(
    enum onk_lexicon_t cmp,
    enum onk_lexicon_t *arr,
    uint16_t narr)
{
  for (uint16_t i=0; narr > i; i++)
    if(arr[i] == 0) break;
    else if (arr[i] == cmp)
      return i;
  return 0;
}

uint16_t onk_tokarr_contains(
    enum onk_lexicon_t cmp,
    struct onk_token_t *arr,
    uint16_t narr)
{
  for (uint16_t i=0; narr > i; i++)
    if (arr[i].type == cmp)
      return i;
  return 0;
}

uint16_t onk_tokarr_len(struct onk_token_t *arr)
{
  for (uint16_t i=0; UINT16_MAX > i; i++)
    if (arr[i].type == 0)
      return i;
  return 0;
}

uint16_t onk_lexarr_len(enum onk_lexicon_t *arr)
{
  for (uint16_t i=0; UINT16_MAX > i; i++)
    if (arr[i] == 0)
      return i;
  return 0;
}


/*
    Inverts brace characters into their counter parts.
    example
       input:"(" - outputs:")"
       input:"]" - output:"["
*/
enum onk_lexicon_t onk_invert_brace(enum onk_lexicon_t token) {
    switch (token) {
        case ONK_PARAM_OPEN_TOKEN: return ONK_PARAM_CLOSE_TOKEN;
        case ONK_PARAM_CLOSE_TOKEN: return ONK_PARAM_OPEN_TOKEN;
        case ONK_BRACE_OPEN_TOKEN: return ONK_BRACE_CLOSE_TOKEN;
        case ONK_BRACE_CLOSE_TOKEN: return ONK_BRACE_OPEN_TOKEN;
        case ONK_BRACKET_CLOSE_TOKEN: return ONK_BRACKET_OPEN_TOKEN;
        case ONK_BRACKET_OPEN_TOKEN: return ONK_BRACKET_CLOSE_TOKEN;
        case ONK_HASHMAP_LITERAL_START_TOKEN: return ONK_BRACE_CLOSE_TOKEN;
        default: return ONK_UNDEFINED_TOKEN;
    }
}

const char * onk_ptoken(enum onk_lexicon_t t) {
    switch (t) {
        case ONK_INTEGER_TOKEN: return "integer";
        case ONK_WORD_TOKEN: return "word";
        case ONK_CHAR_TOKEN: return "char";
        case ONK_NULL_TOKEN: return "nulltoken";
        case ONK_WHITESPACE_TOKEN: return "whitespace";
        case ONK_BRACE_OPEN_TOKEN: return "brace_open";
        case ONK_BRACE_CLOSE_TOKEN: return "brace_close";
        case ONK_PARAM_OPEN_TOKEN: return "param_open";
        case ONK_PARAM_CLOSE_TOKEN: return "param_close";
        case ONK_COMMA_TOKEN: return "comma";
        case ONK_DIGIT_TOKEN: return "digit";
        case ONK_DOUBLE_QUOTE_TOKEN: return "d_quote";
        case ONK_EQUAL_TOKEN: return "eq";
        case ONK_ADD_TOKEN: return "add";
        case ONK_MUL_TOKEN: return "multiply";
        case ONK_DIV_TOKEN: return "divide";
        case ONK_GT_TOKEN: return "greater than";
        case ONK_LT_TOKEN: return "less than";
        case ONK_NOT_EQL_TOKEN: return "is not eq";
        case ONK_ISEQL_TOKEN: return "is eq";
        case ONK_GT_EQL_TOKEN: return "greater than or eq";
        case ONK_LT_EQL_TOKEN: return "less than or eq";
        case ONK_POW_TOKEN: return "exponent";
        case ONK_PLUSEQ_TOKEN: return "plus eq";
        case ONK_MINUS_EQL_TOKEN: return "minus eq";
        case ONK_MOD_TOKEN: return "modolus";
        case ONK_SUB_TOKEN: return "sub";
        case ONK_COLON_TOKEN: return "colon";
        case ONK_SEMICOLON_TOKEN: return "semi-colon";
        case ONK_STRING_LITERAL_TOKEN: return "str_literal";
        case ONK_AMPER_TOKEN: return "&";
        case ONK_PIPE_TOKEN: return "pipe";
        case ONK_AND_TOKEN: return "and";
        case ONK_OR_TOKEN: return "or";
        case ONK_SHR_TOKEN: return "shr";
        case ONK_SHL_TOKEN: return "shl";
        case ONK_BIT_OR_EQL: return "bit or eql";
        case ONK_BIT_AND_EQL: return "bit and eql";
        case ONK_BIT_NOT_EQL: return "bit not eql";
        case ONK_TILDE_TOKEN: return "bit not";
        case ONK_FROM_TOKEN: return "from";
        case ONK_UNDERSCORE_TOKEN: return "underscore";
        case ONK_NOT_TOKEN: return "not";
        case POUND: return "pound";
        case ONK_IF_TOKEN: return "'if";
        case ONK_ELSE_TOKEN: return "'else'";
        case ONK_IMPL_TOKEN: return "'impl'";
        case ONK_DEF_TOKEN: return "'def'";
        case ONK_RETURN_TOKEN: return "'return'";
        //case ONK_AS_TOKEN: return "'as'";
        case ONK_IMPORT_TOKEN: return "'import'";
        case ONK_COMMENT_TOKEN: return "comment";
        case ONK_UNDEFINED_TOKEN: return "undef";
        case ONK_DOT_TOKEN: return "dot";

        case ONK_EOF_TOKEN: return "EOF";

        default: return "ONK_PTOKEN_UNKNOWN_TOKEN";
    };
}
