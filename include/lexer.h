#ifndef _HEADER__LEXER__
#define _HEADER__LEXER__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "clonk.h"

//#include "utils/vec.h"

#define ONK_ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"

enum onk_lexicon_t {
    /* end of file */
    ONK_EOFT = 255,

    /********************/
    /* Transition token */
    /********************/
    __MARKER_TRANSITION_START,

    _ONK_GT_TRANSMISSION_TOKEN,
    /* -123 or -= */
    _ONK_SUB_TRANSMISSION_TOKEN,

    /* may turn into BOREQL or PIPEOP or OR */

    _ONK_AMPER_TRANSMISSION_TOKEN,

    /*  */
    _ONK_LT_TRANSMISSION_TOKEN,

    __MARKER_TRANSITION_END,
    /**********************/
    /* Single byte tokens */
    /**********************/

    __MARKER_GROUP_START,
    onk_tuple_group_token, // (x,x)

    onk_list_group_token,  // [x,x]

    // TODO: produce this
    onk_idx_group_token, // [x:x]

    onk_partial_brace_group_token, // { - unknown type yet
    onk_map_group_token,   // {a:x, b:x}
    onk_code_group_token,  // {x; x;} or {x; x}

    //TODO: make strict mode in predict.c
    // for structure construction
    // ONK_WORD_TOKEN { ONK_WORD_TOKEN = EXPR, ...};
    onk_struct_group_token,// Name {a=b, y=z};

    __MARKER_GROUP_END,

    ONK_TOKEN_UNDEFINED = 1,

    ONK_NULL_TOKEN,

    ONK_WHITESPACE_TOKEN,
    ONK_NEWLINE_TOKEN,
    ONK_COMMENT_TOKEN,

    /*
    ** braces start
    */

    __MARKER_OPEN_BRACE_START,

    /* [ */
    ONK_BRACKET_OPEN_TOKEN,

    /* { */
    ONK_BRACE_OPEN_TOKEN,
    
    /* ( */
    ONK_PARAM_OPEN_TOKEN,
    __MARKER_OPEN_BRACE_END,


    __MARKER_CLOSE_BRACE_START,
    /* ] */
    ONK_BRACKET_CLOSE_TOKEN,

    /* } */
    ONK_BRACE_CLOSE_TOKEN,
    
    /* ) */
    ONK_PARAM_CLOSE_TOKEN,
    __MARKER_CLOSE_BRACE_END,
    __MARKER_UPGRADE_DATA_START,

    /* " */
    ONK_DOUBLE_QUOTE_TOKEN,

    /* ' */
    ONK_SINGLE_QUOTE_TOKEN,

    /* # */
    POUND,

     /* _ */
    ONK_UNDERSCORE_TOKEN,

    /* a-zA-Z */
    ONK_CHAR_TOKEN,

    /* 0-9 single digit */
    ONK_DIGIT_TOKEN,
    __MARKER_UPGRADE_DATA_END,
    __MARKER_DELIM_START,
    /* , */
    ONK_COMMA_TOKEN,

    /* : */
    ONK_COLON_TOKEN,

    /* ; */
    ONK_SEMICOLON_TOKEN,
    __MARKER_DELIM_END,

    /* \ */
    ONK_BACKSLASH_TOKEN,

    /* @ */
    ONK_ATSYM__TOKEN,
    /*********************/
    /* multi byte tokens */
    /*********************/
    __MARKER_UNIT_START,

    /*  [NUM, ..] ONK_WHITESPACE_TOKEN|ONK_SEMICOLON_TOKEN    */
    /* // 20_392  */
    ONK_INTEGER_TOKEN,

    /* [ONK_CHAR_TOKENACTER, ..] ONK_WHITESPACE_TOKEN|ONK_SEMICOLON_TOKEN */
    /* something */
    ONK_WORD_TOKEN,

    ONK_FROM_LOCATION_TOKEN,

    /* [QUOTE, ... QUOTE] */
    /* something */
    ONK_STRING_LITERAL_TOKEN,

    ONK_KEYWORD_TOKEN,
    __MARKER_UNIT_END,

    __MARKER_OP_START,
    __MARKER_UNARY_START,
    /* ~ */
    ONK_TILDE_TOKEN,

    /* ! */
    ONK_NOT_TOKEN,
    __MARKER_UNARY_END,
    __MARKER_BIN_START,

    /* % */
    ONK_MOD_TOKEN,

    /* . */
    ONK_DOT_TOKEN,

    /* * */
    ONK_MUL_TOKEN,

    /* / */
    ONK_DIV_TOKEN,

    /* ^ */
    ONK_POW_TOKEN,

    __MARKER_UPGRADE_OP_START,
    /* + */
    ONK_ADD_TOKEN,

    /* - */
    ONK_SUB_TOKEN,

    /* | */
    PIPE,

    /* & */
    AMPER,

    /* > */
    GT,

    /* < */
    LT,

    __MARKER_ASN_START,
    /* = */
    EQUAL,
    __MARKER_UPGRADE_OP_END,

    __MARKER_COMPOUND_BIN_START,

    /* |= */
    BOREQL,

    /* &= */
    BANDEQL,

    /* ~= */
    BNEQL,

     /* += */
    PLUSEQ,

    /* -= */
    MINUSEQ,
    __MARKER_ASN_END,

    /* .. */
    /* ELLISPES */

    /* >> */
    SHR,

    /* << */
    SHL,

    /* || */
    OR,

    /* && */
    AND,

    /* >= */
    GTEQ,

    /* <= */
    LTEQ,

    /* == */
    ISEQL,

    /* != */
    ISNEQL,

    __MARKER_COMPOUND_BIN_END,

    /* in */
    IN,

    __MARKER_BIN_END,
    __MARKER_GROUP_OP_START,
    _IdxAccess,
    Apply,

    //(Name (field val structG(2)) structInit)
    //StructBlock, // name = Name {field_1 = a, field_2 = "b"};
    StructInit,

    IfCond,
    IfBody,
    DefSign,
    DefBody,

    ForParams, // ((i, i2 g(2)) forparams) (a b g(2)) forbody
    ForBody,

    WhileCond,
    WhileBody,

    /* static */
    /* STATIC, */

    /* const */
    /* CONST, */

    /* extern */
    /* EXTERN, */
    
    /* as */
    //AS,

    __MARKER_KEYONK_WORD_TOKEN_START,
    /* struct A {} */
    STRUCT,

    /* impl A {} */
    IMPL,

    /* return */
    RETURN,

    /* import */
    IMPORT,

    /* from */
    FROM,

    __MARKER_GROUP_OP_END,
    __MARKER_OP_END,


    TRUE,

    FALSE,

    /* if */
    IF,

    /* else */
    ELSE,

    /* def */
    FUNC_DEF,

    FOR,
    WHILE,

    __MARKER_KEYONK_WORD_TOKEN_END

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

    // struct Foo {f: v}
    // Foo (f v mapG(2)) structDeclare

};


/*
    onk_token_ts reference symbols derived from the 
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
struct onk_token_t {
    uint16_t start;
    uint16_t end;
    uint16_t seq;
    enum onk_lexicon_t type;
};

/* sequence of tokens */
struct onk_token_span_t {
    struct onk_token_t start;
    struct onk_token_t end;
};

enum onk_selection_t {
  Scalar,
  Union
};

/*
** onk_token_selection_t describes 1 or more tokens,
** either as a sequencial span
** of token, or as an individual token.
*/
struct onk_token_selection_t {
    enum onk_selection_t type;
    union {
        struct onk_token_t scalar_t;
        struct onk_token_span_t union_t;
    } token;
};

/* Output of lexer function */
struct onk_lexer_output_t {
    const char * src_code;
    uint16_t src_code_sz;

    /* Vec<onk_token_t> */
    const struct Vec tokens;

    /* Vec<onk_lexer_error_t> */
    const struct Vec errors;
};

enum onk_lexer_errno {
    lex_err_missing_end_quote,
    lex_err_non_ascii_token,
    lex_err_input_too_big
};

struct onk_lexer_error_t {
    enum onk_lexer_errno type;
    union {
        struct onk_token_t bad_str;
        struct onk_token_t non_ascii_token;
        //struct UnexpectedErrT unexpected_tok;
    } type_data;
};

struct onk_lexer_input_t {
    const char *src_code;
    struct Vec tokens;
    bool add_eoft;
};

int8_t onk_tokenize(
    struct onk_lexer_input_t *in,
    struct onk_lexer_output_t *out
);

/**
 * returns true if token is of type ONK_COLON_TOKEN or ONK_COMMA_TOKEN
 * @param token
 * @return bool
 */
bool onk_is_tok_delimiter(enum onk_lexicon_t token);


/**
 * Return the parameter `token` into its inverse token type. 
 * parameter `token` is expected to be OPEN_BRACE 
 * CLOSE_BRACE, OPEN_BRACKET, CLOSE_BRACKET
 * OPEN_PARAM, CLOSE_PARAM.
 * All other tokens will return UNDEFINED
 *
 * @param token
 * @return enum onk_lexicon_t
 */
enum onk_lexicon_t invert_brace_tok_ty(enum onk_lexicon_t token);


/**
 * Check if the parameter `token` is
 * LT, GT, ISEQL, LTEQ, GTEQ
 * @param token
 * @return bool
 */
//bool is_cmp_operator(enum onk_lexicon_t compound_token);


/**
 * checks if `cmp` is inside of `buffer`
 *
 * @param cmp item being searched
 * @param buffer collection to be searched
 * @return bool
 */
uint8_t onk_eq_any_tok(enum onk_lexicon_t cmp, enum onk_lexicon_t *buffer);


/**
 * Check if the parameter `token` is equal the token type 
 *   ONK_ADD_TOKEN, ONK_SUB_TOKEN, ONK_DIV_TOKEN, ONK_MOD_TOKEN, ONK_MUL_TOKEN,
 *   AND, OR, ACCESS, ONK_DOT_TOKEN, 
 *   ONK_POW_TOKEN, LT, GT, ISEQL,
 *   LTEQ, GTEQ, EQUAL, PLUSEQ
 *   MINUSEQ, ONK_NOT_TOKEN
 * @param token
 * @return bool
 */
bool onk_is_tok_operator(enum onk_lexicon_t compound_token);


/**
 * Check if the parameter `token` is equal the token type 
 *   CLOSING_PARAM, CLOSING_BRACE, CLOSING_BRACKET
 * @param token
 * @return bool
 */
bool onk_is_tok_close_brace(enum onk_lexicon_t token);


/**
 * Check if the parameter `token` is equal the token type 
 *   OPEN_PARAM, OPEN_BRACE, OPEN_BRACKET
 * @param token
 * @return bool
 */
bool onk_is_tok_open_brace(enum onk_lexicon_t token); 

/**
 * 
 * Checks if a byte is prefixed with utf
 * @param token
 * @return bool
 */
bool onk_is_utf_byte(char ch);


/**
 * Check if the token given is a keyword
 *
 * @param token
 * @return bool
 */
bool onk_is_tok_keyword(enum onk_lexicon_t token);

/**
 * Checks if token is an ONK_INTEGER_TOKEN, and is a negative notation.
 *
 * @param token token to compare
 * @param src source code
 *
 * @return bool
 */
bool onk_is_int_tok_negative(const char *src, struct onk_token_t *token);

/**
 * Checks if token is a unitary expression
 *
 * @param tok token to compare
 *
 * @return bool
 */
bool onk_is_tok_operator(enum onk_lexicon_t token);


/**
 * Checks if token is a unitary expression
 *
 * @param tok token to compare
 *
 * @return bool
*/
bool onk_is_tok_group_modifier(enum onk_lexicon_t);

/**
 * Checks if token is a unitary expression
 *
 * @param tok token to compare
 *
 * @return bool
*/
const char * onk_ptoken(enum onk_lexicon_t t);

/**
 * Checks if token is a unitary expression
 *
 * @param tok token to compare
 *
 * @return bool
 */
bool onk_is_tok_unit(enum onk_lexicon_t tok);


/**
 * Checks if token is a grouping token
 * These tokens are suffixed with "Group"
 *
 * @param tok token to compare
 *
 * @return bool
 */
bool is_group(enum onk_lexicon_t tok);

#endif

