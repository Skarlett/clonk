#include <stdbool.h>
#include "lexer.h"

#define BRACE_BUFFER_SZ 256

bool is_delimiter(enum Lexicon token) {
#if OPTIMIZE
    return COLON <= token && SEMICOLON >= token;
#else
    return token == COLON
        || token == COMMA
        || token == SEMICOLON;
#endif
}

bool is_close_brace(enum Lexicon token)
{
#if OPTIMIZE
  return token >= BRACKET_CLOSE && PARAM_CLOSE >= token;
#else
    return (token == PARAM_CLOSE
    || token == BRACE_CLOSE  
    || token == BRACKET_CLOSE);
#endif
}

bool is_open_brace(enum Lexicon token) {
#if OPTIMIZE
    return token >= BRACKET_OPEN  && BRACE_OPEN >= token;
#else
    return (token == PARAM_OPEN
    || token == BRACE_OPEN  
    || token == BRACKET_OPEN);
#endif
}

bool is_unit(enum Lexicon token) {
#if OPTIMIZE
  return token >= INTEGER && NULL_KEYWORD >= token;
#else
    return (token == WORD
        || token == INTEGER 
        || token == STRING_LITERAL
        //|| token == NULLTOKEN
        || token == NULL_KEYWORD
    );
#endif
}

bool is_asn_operator(enum Lexicon token) {
#if OPTIMIZE
    return token >= EQUAL && MINUSEQ >= token;
#else
    return (
        token == EQUAL
        || token == MINUSEQ
        || token == PLUSEQ
        || token == BOREQL
        || token == BANDEQL
        || token == BNEQL
    );
#endif
}

/*
   returns bool if token is a binary operator
*/
bool is_operator(enum Lexicon token) {
#if OPTIMIZE
    return token >= NOT && ISNEQL >= token;
#else
    return (
        /* comparison operators */
        token == ISEQL
        || token == ISNEQL 
        || token == GTEQ 
        || token == LTEQ
        || token == GT
        || token == LT
        /* logic operators */
        || token == OR
        || token == AND
        || token == NOT
        /* arithmetic */
        || token == ADD
        || token == SUB
        || token == MUL
        || token == POW
        || token == NOT
        || token == MOD
        || token == DIV
        || token == DOT
        /* assignments */
        || token == EQUAL
        || token == MINUSEQ
        || token == PLUSEQ
        /* bitwise operations */
        || token == BOREQL
        || token == BANDEQL
        || token == PIPE
        || token == AMPER
        || token == SHL
        || token == SHR
        /* experimental */
        //|| token == PIPEOP
    );
#endif
}

/* null delimitated */
uint8_t eq_any_tok(enum Lexicon cmp, enum Lexicon buffer[]) {
  for (uint16_t i=0 ;; i++)
    if(buffer[i] == 0) break;
    else if (buffer[i] == cmp)
      return i;
  
  return 0;
}

/* is character utf encoded */
bool is_utf(char ch) {
    return ((unsigned char)ch >= 0x80);
}

/*
    Inverts brace characters into their counter parts.
    example
       input:"(" - outputs:")"
       input:"]" - output:"["
*/
enum Lexicon invert_brace_tok_ty(enum Lexicon token) {
    switch (token) {
        case PARAM_OPEN: return PARAM_CLOSE;
        case PARAM_CLOSE: return PARAM_OPEN;
        case BRACE_OPEN: return BRACE_CLOSE;
        case BRACE_CLOSE: return BRACE_OPEN;
        case BRACKET_CLOSE: return BRACKET_OPEN;
        case BRACKET_OPEN: return BRACKET_CLOSE;
        default: return TOKEN_UNDEFINED;
    }
}

int8_t inner_balance(enum Lexicon tokens[], uint16_t *tokens_ctr, enum Lexicon current) {
    //TODO: check for int overflow & buffer
    enum Lexicon inverted;

    if (is_close_brace(current))
    {
        inverted = invert_brace_tok_ty(current);
        if (inverted == TOKEN_UNDEFINED)
          return -1;
        
       /*
            return 1 to stop iteration, 
            and return `is_balanced` as false (0)
        */    
        else if (*tokens_ctr <= 0)
          return 1;

        else if (tokens[*tokens_ctr - 1] == inverted)
          *tokens_ctr -= 1;
    }

    else if (is_open_brace(current)) {
        if (*tokens_ctr >= BRACE_BUFFER_SZ) {
            return -1;
        }
        
        *tokens_ctr += 1;
        tokens[(*tokens_ctr)-1] = current;
    }

    return 0;
}

/* 
 *  function determines if an expression is unbalanced.
 *  an expression can be unbalanced 
 *  if there is a nonmatching `[` / `(` / `{` character
*/
bool is_balanced(struct Token tokens[], uint16_t ntokens) {
    enum Lexicon braces[BRACE_BUFFER_SZ];
    uint16_t braces_ctr = 0;
    int8_t ret;

    for (uint16_t i=0; ntokens > i; i++){
        ret = inner_balance(braces, &braces_ctr, tokens[i].type);
        if (ret == -1)
            return -1;
        
        // immediately unbalanced
        else if (ret == 1)
            return 0;
    }

    return braces_ctr == 0;
}

/* 
    function determines if an expression is unbalanced.
    an expression can be unbalanced 
    if there is a nonmatching `[` / `(` / `{` character
*/
bool is_balanced_by_ref(struct Token *tokens[], uint16_t ntokens) {
    enum Lexicon braces[BRACE_BUFFER_SZ];
    uint16_t braces_ctr = 0;

    int8_t ret;
    
    for (uint16_t i=0; ntokens > i; i++){
        ret = inner_balance(braces, &braces_ctr, tokens[i]->type);
        
        if (ret == -1)
            return -1;
        
        else if (ret == 1)
            return 0;
    }
    
    return braces_ctr == 0;
}

bool is_keyword(enum Lexicon token) {
#if OPTIMIZE
    return token >= RETURN && IMPL >= token;
#else
    return token == RETURN
        || token == IF
        || token == ELSE
        || token == FUNC_DEF
        || token == STRUCT
        || token == IMPL
        || token == FROM
        || token == IMPORT
        || token == FOR
        || token == WHILE;
#endif
}

bool is_num_negative(const char * source, struct Token *token) {
    return token->type == INTEGER && *(source + token->start) == '-';
}

bool is_group(enum Lexicon tok)
{
#if OPTIMIZE
    return token >= TupleGroup && StructGroup >= token;
#else
  return tok == TupleGroup
    || tok == ListGroup
    || tok == IndexGroup
    || tok == MapGroup
    || tok == CodeBlock
    || tok == StructGroup;
#endif
}

bool is_group_modifier(enum Lexicon tok) {
#if OPTIMIZE
    return tok >= STRUCT && WhileBody >= tok;
#else
    return tok == Apply
        || tok == _IdxAccess
        || tok == IfCond
        || tok == IfBody
        || tok == DefSign
        || tok == DefBody
        || tok == ForParams
        || tok == ForBody
        || tok == WhileCond
        || tok == WhileBody
        || tok == IMPORT
        || tok == RETURN
        || tok == StructInit
        || tok == STRUCT
        || tok == IMPL;
#endif
}
