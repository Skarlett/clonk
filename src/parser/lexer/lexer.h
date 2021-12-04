#ifndef _HEADER__LEXER__
#define _HEADER__LEXER__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../prelude.h"
#include "../error.h"


#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"

enum Lexicon {
    // end of file
    EOFT = 255,
    TOKEN_UNDEFINED = 0,
    
    NULLTOKEN,
    
    // Ignore token (whitespace, newline, carriage return)
    WHITESPACE,
    
    NEWLINE,

    // Ignore token (whitespace, newline, carriage return)
    COMMENT,

    // [
    BRACKET_OPEN,

    // ]
    BRACKET_CLOSE,

    // {
    BRACE_OPEN,
    
    // }
    BRACE_CLOSE,
    
    // (
    PARAM_OPEN,
    
    // )
    PARAM_CLOSE,
    
    // !
    NOT,

    // +
    ADD,
    
    // -
    SUB,

    //
    _COMPOUND_GT,

    // >
    GT,

    // >>
    SHR,

    // >=
    GTEQ,

    //
    _COMPOUND_LT,

    // <
    LT,
    
    // <<
    SHL,

    // <=
    LTEQ,

    // *
    MUL,

    // /
    DIV,

    // ^
    POW,

    // %
    MOD,

    // +=
    PLUSEQ,

    // -=
    MINUSEQ,
    
    // =
    EQUAL,

    // ==
    ISEQL,

    // !=
    ISNEQL,

    // may turn into BOREQL or PIPEOP or OR 
    _COMPOUND_PIPE,

    // |
    PIPE,

    // |>
    PIPEOP,

    // |=
    BOREQL,

    // ||
    OR,

    _COMPOUND_AMPER,

    // &
    AMPER,

    // &=
    BANDEQL,

    // &&
    AND,

    // ~
    TILDE,

    // ~=
    BNEQL,

    // "
    QUOTE,
    
    // ;
    SEMICOLON,

    // _
    UNDERSCORE,

    // .
    DOT,

    // #
    POUND,

    // :
    COLON,

    // @
    ATSYM,

    // a-zA-Z
    CHAR,

    // 0-9 *single digit
    DIGIT,
    
    // ,
    COMMA,

    /********* START OF COMPLEX TOKENS ********
    * Complex tokens wont show up in the first round of lexer'ing
    * they're generated from combinations of tokens
    * "fn"
    ********** START OF COMPLEX LEXICONS ********/

    // -123 or -=
    _COMPOUND_SUB,

    // [NUM, ..] WHITESPACE|SEMICOLON   
    // 20392
    INTEGER,

    // [CHARACTER, ..] WHITESPACE|SEMICOLON
    // something
    WORD,

    // [QUOTE, ... QUOTE]
    // something
    STRING_LITERAL,

    // static
    STATIC,

    // const
    CONST,

    // return
    RETURN,
    
    // extern
    EXTERN,
    
    // as
    AS,
    
    // if
    IF,

    // else
    ELSE,

    // def
    FUNC_DEF,

    //import
    IMPORT,
    
    //impl
    IMPL,
    
    /*
        this is a 'pretend' token used 
        internally by expression
    */

    _IdxAccess, 

    // foo(a)(b)
    // foo(a) -> func(b) -> T
    // foo(a)(b) -> ((a foo), b G(2)) DyCall
    // foo(a)(b)(c) -> a foo b G(2) DyCall c G(2) DyCall
    // word|)|]|}(
    Apply,

    /*
      GROUPING token are generated in the expression parser
        
      The GROUPING token is used to track the amount 
      of sub-expressions inside an expression.
        
      - `start` 
          points to its origin grouping token,
          or 0 if not applicable
        
      - `end`
          is the amount of arguments to pop from the stack

      See example expression:
          [1, 2, 3]
      
          where '1', '2', '3' are atomic expressions.
          When grouped together by a comma to become 
          "1,2,3" this is called grouping.

          Groupings may not explicitly 
          point to a brace type if none is present. 
            
          After postfix evaluation, 
          a group token is added into the postfix output.

          1 2 3 GROUP(3)
        
      it's `start` attribute will point to its origin grouping token
      and its `end` attribute will determine the amount of arguments
      it will take off of the stack
    
      tokens of this type will be spawned from a differing source
      than the original token stream.
    */
    TupleGroup, // (x,x)
    ListGroup,  // [x,x]
    MapGroup,   // {x:x}
    SetGroup,   // {x,x}
    
    CodeBlock,  // {x; x;} or {x; x}

    IfCond,
    IfBody,
    DefSign,
    DefBody
};

/*
    Tokens reference symbols derived from the 
    source code given to the interpreter.
    the following fields correlate with the following,
    except when `type` is equal to FNMASK, or COMPOSITE.
    
    `start` correlates to the starting position the token inside the source code string.
    The position is directly indexable against its source (char *line).

    `end` correlates to the ending position of the token inside the source code string.
    The position is directly indexable against its source (char *line). 

    `type` defines what kind of token it is.

    In the case of `type` being of MARKER:
        `start` correlates to the starting position of the token instead the token array.
        The position is directly indexable against its source (tokens[]).

        `end` correlates to the ending position of the token instead the token array.
        The position is directly indexable against its source (tokens[]).
*/
struct Token {
    uint16_t start;
    uint16_t end;
    enum Lexicon type;
};

int8_t tokenize(
    const char *line,
    struct Token tokens[],
    uint16_t *token_ctr,
    uint16_t token_sz,
    struct CompileTimeError *error
);

/**
 * returns true if token is of type COLON or COMMA
 * @param token
 * @return bool
 */
bool is_delimiter(enum Lexicon token);

/**
 * Return the parameter `token` into its inverse token type. 
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
 * Check if the parameter `token` is
 * LT, GT, ISEQL, LTEQ, GTEQ
 * @param token
 * @return bool
 */
bool is_cmp_operator(enum Lexicon compound_token);


/**
 * checks if `cmp` is inside of `buffer`
 *
 * @param cmp item being searched
 * @param buffer collection to be searched
 * @return bool
 */
bool contains_tok(enum Lexicon cmp, enum Lexicon *buffer);


/**
 * Check if the parameter `token` is equal the token type 
 *   ADD, SUB, DIV, MOD, MUL,
 *   AND, OR, ACCESS, DOT, 
 *   POW, LT, GT, ISEQL,
 *   LTEQ, GTEQ, EQUAL, PLUSEQ
 *   MINUSEQ, NOT
 * @param token
 * @return bool
 */
bool is_operator(enum Lexicon compound_token);


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
bool is_balanced(struct Token tokens[], uint16_t ntokens);


/**
 * Checks if token stream is balanced by reference. see (src/parser/lexer/helpers.h#is_balance)
 *
 * @param tokens stream of referenced tokens
 * @param ntokens amount of tokens to read
 *
 * @return bool
 */
bool is_balanced_by_ref(struct Token *tokens[], uint16_t ntokens);


/**
 * Checks if token is an INTEGER, and is a negative notation.
 *
 * @param tokens stream of tokens
 *
 * @return bool
 */
bool is_num_negative(const char *src, struct Token *token);


bool is_operator(enum Lexicon token);

const char * ptoken(enum Lexicon t);

char invert_brace_char(char brace);

#endif

