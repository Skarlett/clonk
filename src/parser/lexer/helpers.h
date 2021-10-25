#ifndef _HEADER__LEXER_HELPER__
#define _HEADER__LEXER_HELPER__

#include <stdint.h>
#include "../../prelude.h"
#include "lexer.h"
#include <stdbool.h>


/**
 * Convert the parameter `token` into its inverse token type. 
 * parameter `token` is expected to be OPEN_BRACE 
 * CLOSE_BRACE, OPEN_BRACKET, CLOSE_BRACKET
 * OPEN_PARAM, CLOSE_PARAM.
 * All other tokens will return UNDEFINED
 *
 * @param token
 * @return enum Lexicon
 */
enum Lexicon invert_brace_tok_ty(enum Lexicon token);


/**
 * Check if the parameter `token` is equal the token type 
 * 
 * @param token
 * @return bool
 */
bool is_fncall(struct Token tokens[], usize ntokens);


/**
 * Check if the parameter `token` is
 * LT, GT, ISEQL, LTEQ, GTEQ
 * @param token
 * @return bool
 */
bool is_cmp_operator(enum Lexicon compound_token);


/**
 * Check if the parameter `token` is equal the token type 
 *   ADD, SUB, DIV, MOD, MUL,
 *   AND, OR, ACCESS, DOT, 
 *   POW, LT, GT, ISEQL,
 *   LTEQ, GTEQ
 * @param token
 * @return bool
 */
bool is_bin_operator(enum Lexicon compound_token);


/**
 * Check if the parameter `token` is equal the token type 
 *   CLOSING_PARAM, CLOSING_BRACE, CLOSING_BRACKET
 * @param token
 * @return bool
 */
bool is_close_brace(enum Lexicon token);


/**
 * Check if the parameter `token` is equal the token type 
 *   OPEN_PARAM, OPEN_BRACE, OPEN_BRACKET
 * @param token
 * @return bool
 */
bool is_open_brace(enum Lexicon token); 


/**
 * Check if parameter `token` is
 * symbolic or literal data.
 * returns `true` if `token` is
 * STRING_LITERAL, INTEGER or
 * WORD.
 * @param token
 * @return bool
 */
bool is_symbolic_data(enum Lexicon token);


/**
 * 
 * Checks if a byte is prefixed with utf
 * @param token
 * @return bool
 */
bool is_utf(char ch);


/**
 * Check if the token given is a keyword
 *
 * @param token
 * @return bool
 */
bool is_keyword(enum Lexicon token);


/**
 * Checks if token stream is balanced.
 * To be balanced is every brace opening,
 * having a pairing brace closing token 
 * following it eventually.
 * The follow are examples:
 *
 *   (a + b + (2 + 5)) Is balanced
 *   (a + b            Is unbalanced.
 *
 * @param tokens stream of tokens
 * @param ntokens amount of tokens to read
 *
 * @return bool
 */
bool is_balanced(struct Token tokens[], usize ntokens);


/**
 * Checks if token stream is balanced by reference. see (src/parser/lexer/helpers.h#is_balance)
 *
 * @param tokens stream of referenced tokens
 * @param ntokens amount of tokens to read
 *
 * @return bool
 */
bool is_balanced_by_ref(struct Token *tokens[], usize ntokens);


/**
 * Checks if token is an INTEGER, and is a negative notation.
 *
 * @param tokens stream of tokens
 *
 * @return bool
 */
bool is_num_negative(struct Token *token);

#endif
