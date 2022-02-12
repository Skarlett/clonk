#ifndef _HEADER__LEXER__
#define _HEADER__LEXER__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "../../utils/vec.h"

#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"

enum Lexicon {
    /* end of file */
    EOFT = 255,

    /********************/
    /* Transition token */
    /********************/

    _COMPOUND_GT,
    /* -123 or -= */
    _COMPOUND_SUB,

    /* may turn into BOREQL or PIPEOP or OR */
    _COMPOUND_PIPE,

    _COMPOUND_AMPER,

    /*  */
    _COMPOUND_LT,

    /**********************/
    /* Single byte tokens */
    /**********************/

    TOKEN_UNDEFINED = 1,
    
    NULLTOKEN,
    
    WHITESPACE,
    
    NEWLINE,

    COMMENT,

    /* [ */
    BRACKET_OPEN,

    /* ] */
    BRACKET_CLOSE,

    /* { */
    BRACE_OPEN,
    
    /* } */
    BRACE_CLOSE,
    
    /* ( */
    PARAM_OPEN,
    
    /* ) */
    PARAM_CLOSE,
    
    /* ! */
    NOT,

    /* + */
    ADD,
    
    /* - */
    SUB,

    /* > */
    GT,

    /* < */
    LT,

    /* * */
    MUL,

    /* / */
    DIV,

    /* ^ */
    POW,

    /* % */
    MOD,

    /* = */
    EQUAL,

    /* | */
    PIPE,

    /* & */
    AMPER,

    /* ~ */
    TILDE,

    /* " */
    D_QUOTE,

    /* ' */
    S_QUOTE,
    
    /* ; */
    SEMICOLON,

     /* _ */
    UNDERSCORE,

    /* . */
    DOT,

    /* # */
    POUND,

    /* : */
    COLON,

    /* @ */
    ATSYM,

    /* a-zA-Z */
    CHAR,

    /* 0-9 *single digit */
    DIGIT,
    
    /* , */
    COMMA,

    /* \ */
    BACKSLASH,

    /*********************/
    /* multi byte tokens */
    /*********************/

    /*  [NUM, ..] WHITESPACE|SEMICOLON    */
    /* // 20_392  */
    INTEGER,

    /* [CHARACTER, ..] WHITESPACE|SEMICOLON */
    /* something */
    WORD,

    /* [QUOTE, ... QUOTE] */
    /* something */
    STRING_LITERAL,

    /* >> */
    SHR,

    /* >= */
    GTEQ,

    /* << */
    SHL,

    /* <= */
    LTEQ,

    /* += */
    PLUSEQ,

    /* -= */
    MINUSEQ,

    /* .. */
    /* ELLISPES */

    /* |> */
    PIPEOP,

    /* |= */
    BOREQL,

    /* || */
    OR,

    /* &= */
    BANDEQL,

    /* == */
    ISEQL,

    /* ~= */
    BNEQL,

    /* != */
    ISNEQL,
    
    /* && */
    AND,

    /* static */
    /* STATIC, */

    /* const */
    /* CONST, */

    /* return */
    RETURN,
    
    /* extern */
    /* EXTERN, */
    
    /* as */
    /* AS, */
    
    /* if */
    IF,

    /* else */
    ELSE,

    /* def */
    FUNC_DEF,

    /* import */
    IMPORT,

    /* from */
    FROM,

    FROM_LOCATION,

    FOR,
    WHILE,

    /* struct A {} */
    STRUCT,

    /* impl A {} */
    IMPL,

    /*********************************/
    /* generated outside lexer stage */
    /*********************************/

    _IdxAccess,
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
    IndexGroup, // [x:x]

    PartialBrace, // { - unknown type yet
    MapGroup,   // {x:x}
    CodeBlock,  // {x; x;} or {x; x}


    IfCond,
    IfBody,
    DefSign,
    DefBody,

    ForParams, // ((i, i2 g(2)) forparams) (a b g(2)) forbody
    ForBody,

    WhileCond,
    WhileBody
};


#define _EX_BIN_OPERATOR \
    ADD, MUL, SUB, DIV, POW, MOD, \
    ISEQL, ISNEQL, LT, LTEQ, OR, AND,\
    GTEQ, GT, SHL, SHR, AMPER, PIPE, PIPEOP

#define _EX_KEYWORDS_BEGINNING \
    IF, FOR, WHILE, FUNC_DEF, RETURN

#define _EX_KEYWORDS_ALL 0

#define _EX_UNARY_OPERATOR \
    TILDE, NOT

#define _EX_DELIM \
    COMMA, COLON, SEMICOLON

#define _EX_ASN_OPERATOR \
    EQUAL, PLUSEQ, MINUSEQ, \
    BANDEQL, BOREQL, BNEQL

#define _EX_OPEN_BRACE \
    PARAM_OPEN, BRACE_OPEN, BRACKET_OPEN

#define _EX_CLOSE_BRACE \
    PARAM_CLOSE, BRACE_CLOSE, BRACKET_CLOSE

#define _EX_DATA \
    STRING_LITERAL, WORD, INTEGER

#define _EX_EXPR \
    _EX_DATA, _EX_OPEN_BRACE, _EX_UNARY_OPERATOR

/*
    Tokens reference symbols derived from the 
    source code given to the interpreter.
    the following fields correlate with the following,
    except when `type` is equal to FNMASK, or COMPOSITE.
    
    `start` correlates to the starting position the token inside the source code string.
    The position is directly indexable against its source (char *src_code).

    `end` correlates to the ending position of the token inside the source code string.
    The position is directly indexable against its source (char *src_code). 

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
    uint16_t seq;
    enum Lexicon type;
};

/* sequence of tokens */
struct TokenSpan {
    struct Token start;
    struct Token end;
};

enum Selection_t {
  Scalar,
  Union
};

/*
** TokenSelection describes 1 or more tokens,
** either as a sequencial span
** of token, or as an individual token.
*/
struct TokenSelection {
    enum Selection_t type;
    union {
        struct Token scalar_t;
        struct TokenSpan union_t;
    } token;
};

/* Output of lexer function */
struct LexerOutput {
    const char * src_code;
    uint16_t src_code_sz;

    /* Vec<Token> */
    const struct Vec tokens;

    /* Vec<LexerError> */
    const struct Vec errors;
};

enum LexerError_t {
    lex_err_missing_end_quote,
    lex_err_non_ascii_token,
    lex_err_input_too_big
};

struct LexerError {
    enum LexerError_t type;
    union {
        struct Token bad_str;
        struct Token non_ascii_token;
        //struct UnexpectedErrT unexpected_tok;
    } type_data;
};

struct LexerInput {
    const char *src_code;
    struct Vec tokens;
    bool add_eoft;
};

int8_t tokenize(
    struct LexerInput *in,
    struct LexerOutput *out
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
uint8_t eq_any_tok(enum Lexicon cmp, enum Lexicon *buffer);


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
 *    a + b + (2 + 5)  Is balanced
 *   (a + b            Is unbalanced.
 *
 * @param tokens array of tokens
 * @param ntokens amount of tokens to read
 *
 * @return bool
 */
bool is_balanced(struct Token tokens[], uint16_t ntokens);


/**
 * Checks if token stream is balanced by reference. see (src/parser/lexer/helpers.h#is_balance)
 *
 * @param tokens array of referenced tokens
 * @param ntokens amount of tokens to read
 *
 * @return bool
 */
bool is_balanced_by_ref(struct Token *tokens[], uint16_t ntokens);


/**
 * Checks if token is an INTEGER, and is a negative notation.
 *
 * @param token token to compare
 * @param src source code
 *
 * @return bool
 */
bool is_num_negative(const char *src, struct Token *token);

/**
 * Checks if token is a unitary expression
 *
 * @param tok token to compare
 *
 * @return bool
 */
bool is_operator(enum Lexicon token);


/**
 * Checks if token is a unitary expression
 *
 * @param tok token to compare
 *
 * @return bool
*/
bool is_group_modifier(enum Lexicon);

/**
 * Checks if token is a unitary expression
 *
 * @param tok token to compare
 *
 * @return bool
*/
const char * ptoken(enum Lexicon t);


/**
 * Checks if token is a unitary expression
 *
 * @param tok token to compare
 *
 * @return bool
 */
 /*char invert_brace_char(char brace);*/

/**
 * Checks if token is a unitary expression
 *
 * @param tok token to compare
 *
 * @return bool
 */
bool is_unit(enum Lexicon tok);


/**
 * Checks if token is a grouping token
 * These tokens are suffixed with "Group"
 *
 * @param tok token to compare
 *
 * @return bool
 */
bool is_group(enum Lexicon tok);
#endif

