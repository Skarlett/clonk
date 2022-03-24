#ifndef _HEADER__LEXER__
#define _HEADER__LEXER__

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "clonk.h"
#include "onkstd/vec.h"

enum onk_lexicon_t {
    /* end of file */
    ONK_EOFT,

    __ONK_MARKER_ILLEGAL_INPUT_START,
    /********************/
    /* Transition token */
    /********************/
    __ONK_MARKER_TRANSITION_START,

    _ONK_GT_TRANSMISSION_TOKEN,
    /* -123 or -= */
    _ONK_SUB_TRANSMISSION_TOKEN,

    _ONK_PIPE_TRANSMISSION_TOKEN,
    /* may turn into ONK_BIT_OR_EQL or PIPEOP or OR */

    _ONK_AMPER_TRANSMISSION_TOKEN,

    _ONK_DOLLAR_TRANSMISSION_TOKEN,

    /*  */
    _ONK_LT_TRANSMISSION_TOKEN,

    __ONK_MARKER_TRANSITION_END,

    /**********************/
    /* Single byte tokens */
    /**********************/

    __ONK_MARKER_GROUP_START,
    onk_tuple_group_token, // (x,x)

    onk_list_group_token,  // [x,x]

    // TODO: produce this
    onk_idx_group_token, // [x:x]

    onk_map_group_token,   // {a:x, b:x}
    onk_code_group_token,  // {x; x;} or {x; x}

    //TODO: make strict mode in predict.c
    // for structure construction
    // ONK_WORD_TOKEN { ONK_WORD_TOKEN = EXPR, ...};
    onk_struct_group_token,// Name {a=b, y=z};

    __ONK_MARKER_GROUP_END,

    ONK_UNDEFINED_TOKEN,

    __ONK_MARKER_UPGRADE_DATA_START,

    __ONK_MARKER_UNARY_START,
    /* ~ */
    ONK_TILDE_TOKEN,

    /* ! */
    ONK_NOT_TOKEN,
    __ONK_MARKER_UNARY_END,

    /* " */
    ONK_DOUBLE_QUOTE_TOKEN,

    /* ' */
    ONK_SINGLE_QUOTE_TOKEN,

    /* # */
    POUND,

    /* $  */
    ONK_DOLLAR_TOKEN,

    /* _ */
    ONK_UNDERSCORE_TOKEN,

    /* a-zA-Z */
    ONK_CHAR_TOKEN,

    /* 0-9 single digit */
    ONK_DIGIT_TOKEN,
    __ONK_MARKER_UPGRADE_DATA_END,

    /* \ */
    ONK_BACKSLASH_TOKEN,

    /* @ */
    //ONK_ATSYM__TOKEN,

    __ONK_MARKER_ILLEGAL_INPUT_END,
    /*********************/
    /* multi byte tokens */
    /*********************/
    __ONK_MARKER_WHITESPACE_START,
    ONK_WHITESPACE_TOKEN,
    ONK_NEWLINE_TOKEN,
    __ONK_MARKER_WHITESPACE_END,

    ONK_COMMENT_TOKEN,

    __ONK_MARKER_DEFAULT_EXPECT_START,

    __ONK_MARKER_UNIT_START,
    __ONK_MARKER_KEYWORD_DATA_START,
    ONK_TRUE_TOKEN,
    ONK_FALSE_TOKEN,
    ONK_NULL_TOKEN,
    __ONK_MARKER_KEYWORD_DATA_END,

    /* from location */
    ONK_FROM_LOCATION,

    /*  [NUM, ..] ONK_WHITESPACE_TOKEN|ONK_SEMICOLON_TOKEN    */
    /* // 20_392  */
    ONK_INTEGER_TOKEN,

    /* [ONK_CHAR_TOKEN, ..] ONK_WHITESPACE_TOKEN|ONK_SEMICOLON_TOKEN */
    ONK_WORD_TOKEN,

    /* [QUOTE, ... QUOTE] */
    ONK_STRING_LITERAL_TOKEN,
    __ONK_MARKER_UNIT_END,

    __ONK_MARKER_DELIM_START,
    /* , */
    ONK_COMMA_TOKEN,

    /* : */
    ONK_COLON_TOKEN,

    /* ; */
    ONK_SEMICOLON_TOKEN,
    __ONK_MARKER_DELIM_END,

    __ONK_MARKER_BRACE_START,
    __ONK_MARKER_CLOSE_BRACE_START,
    /* ] */
    ONK_BRACKET_CLOSE_TOKEN,

    /* } */
    ONK_BRACE_CLOSE_TOKEN,

    /* ) */
    ONK_PARAM_CLOSE_TOKEN,
    __ONK_MARKER_CLOSE_BRACE_END,

    __ONK_MARKER_OPEN_BRACE_START,

    /* ${ */
    ONK_HASHMAP_LITERAL_START_TOKEN,

    /* [ */
    ONK_BRACKET_OPEN_TOKEN,

    /* { */
    ONK_BRACE_OPEN_TOKEN,

    /* ( */
    ONK_PARAM_OPEN_TOKEN,
    __ONK_MARKER_OPEN_BRACE_END,
    __ONK_MARKER_BRACE_END,

    __ONK_MARKER_OP_START,
    __ONK_MARKER_BIN_START,
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

    __ONK_MARKER_ASN_START,

    __ONK_MARKER_COMPOUND_BIN_START,

    /* |= */
    ONK_BIT_OR_EQL,

    /* &= */
    ONK_BIT_AND_EQL,

    /* ~= */
    ONK_BIT_NOT_EQL,

     /* += */
    ONK_PLUSEQ_TOKEN,

    /* -= */
    ONK_MINUS_EQL_TOKEN,

    __ONK_MARKER_UPGRADE_OP_START,

    /* = */
    ONK_EQUAL_TOKEN,
    __ONK_MARKER_ASN_END,

    /* + */
    ONK_ADD_TOKEN,

    /* - */
    ONK_SUB_TOKEN,

    /* | */
    ONK_PIPE_TOKEN,

    /* & */
    ONK_AMPER_TOKEN,

    /* > */
    ONK_GT_TOKEN,

    /* < */
    ONK_LT_TOKEN,

    /* .. */
    /* ELLISPES */

    /* >> */
    ONK_SHR_TOKEN,

    /* << */
    ONK_SHL_TOKEN,

    /* || */
    ONK_OR_TOKEN,

    /* && */
    ONK_AND_TOKEN,

    /* >= */
    ONK_GT_EQL_TOKEN,

    /* <= */
    ONK_LT_EQL_TOKEN,

    /* == */
    ONK_ISEQL_TOKEN,

    /* != */
    ONK_NOT_EQL_TOKEN,

    __ONK_MARKER_COMPOUND_BIN_END,

    /* in */
    ONK_IN_TOKEN,

    __ONK_MARKER_BIN_END,
    __ONK_MARKER_GROUP_OP_START,
    onk_idx_op_token,
    onk_apply_op_token,

    //(Name (field val structG(2)) structInit)
    //StructBlock, // name = Name {field_1 = a, field_2 = "b"};
    onk_struct_init_op_token,

    onk_ifcond_op_token,
    onk_ifbody_op_token,
    DefSign,
    onk_defbody_op_token,

    onk_for_args_op_token, // ((i, i2 g(2)) forparams) (a b g(2)) forbody
    onk_for_body_op_token,

    onk_while_cond_op_token,
    onk_while_body_op_token,

    __ONK_MARKER_GROUP_OP_END,
    /* static */
    /* STATIC, */

    /* const */
    /* CONST, */

    /* extern */
    /* EXTERN, */
    
    /* as */
    //AS,

    __ONK_MARKER_KEYWORD_BLOCK_START,
    /* struct A {} */
    ONK_STRUCT_TOKEN,

    /* impl A {} */
    ONK_IMPL_TOKEN,

    /* return */
    ONK_RETURN_TOKEN,

    /* import */
    ONK_IMPORT_TOKEN,

    __ONK_MARKER_LOOP_CTL_START,
    /* break */
    ONK_BREAK_TOKEN,

    /* continue */
    ONK_CONTINUE_TOKEN,
    __ONK_MARKER_LOOP_CTL_END,

    /* from */
    ONK_FROM_TOKEN,

    __ONK_MARKER_OP_END,

    /* if */
    ONK_IF_TOKEN,

    /* else */
    ONK_ELSE_TOKEN,

    /* def */
    ONK_DEF_TOKEN,

    ONK_FOR_TOKEN,
    ONK_WHILE_TOKEN,

    __ONK_MARKER_KEYWORD_BLOCK_END,

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
    const struct onk_vec_t tokens;

    /* Vec<onk_lexer_error_t> */
    const struct onk_vec_t errors;
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
    struct onk_vec_t tokens;
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
 * LT, GT, ONK_ISEQL_TOKEN, ONK_LT_EQL_TOKEN, ONK_GT_EQL_TOKEN
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
 * Check if the parameter `token` is an operator
 * @param token
 * @return bool
 */
bool onk_is_tok_operator(enum onk_lexicon_t compound_token);


/**
 * Check if the parameter `token` is an operator
 * @param token
 * @return bool
 */
bool onk_is_tok_unary_operator(enum onk_lexicon_t compound_token);

/**
 * Check if the parameter `token` is equal the token type
 *   CLOSING_PARAM, CLOSING_BRACE, CLOSING_BRACKET
 * @param token
 * @return bool
 */
bool onk_is_tok_brace(enum onk_lexicon_t token);

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


bool onk_is_tok_block_keyword(enum onk_lexicon_t token);

bool onk_is_tok_data_keyword(enum onk_lexicon_t token);

bool onk_is_tok_keyword(enum onk_lexicon_t token);

bool _onk_do_default_expectation(enum onk_lexicon_t token);

bool onk_is_tok_illegal(enum onk_lexicon_t token);

bool onk_is_tok_loopctlkw(enum onk_lexicon_t tok);

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
 * Checks if token is a whitespace
 *
 * @param tok token to compare
 *
 * @return bool
*/
bool onk_is_tok_binop(enum onk_lexicon_t token);

/**
 * Checks if token is a whitespace
 *
 * @param tok token to compare
 *
 * @return bool
*/
bool onk_is_tok_asn_operator(enum onk_lexicon_t token);

/**
 * Checks if token is a whitespace
 *
 * @param tok token to compare
 *
 * @return bool
*/
bool onk_is_tok_unary_operator(enum onk_lexicon_t token);

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
 * Checks if token is a whitespace
 *
 * @param tok token to compare
 *
 * @return bool
*/
bool onk_is_tok_whitespace(enum onk_lexicon_t tok);
/**
 * Checks if token is a grouping token
 * These tokens are suffixed with "Group"
 *
 * @param tok token to compare
 *
 * @return bool
 */
bool _onk_is_group(enum onk_lexicon_t tok);


/*
    Inverts brace characters into their counter parts.
    example
       input:"(" - outputs:")"
       input:"]" - output:"["
*/
enum onk_lexicon_t onk_invert_brace(enum onk_lexicon_t token);

#endif

