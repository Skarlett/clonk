#include <stdbool.h>
#include "lexer.h"
#include "../../prelude.h"

#define BRACE_BUFFER_SZ 256

bool is_delimiter(enum Lexicon token) {
    return token == COLON || token == COMMA;
}

bool is_close_brace(enum Lexicon token) {
    return (token == PARAM_CLOSE 
    || token == BRACE_CLOSE  
    || token == BRACKET_CLOSE);
}

bool is_open_brace(enum Lexicon token) {
    return (token == PARAM_OPEN 
    || token == BRACE_OPEN  
    || token == BRACKET_OPEN);
}

bool is_symbolic_data(enum Lexicon token) {
    return (token == WORD
        || token == INTEGER 
        || token == STRING_LITERAL
        || token == NULLTOKEN
    );
}

bool is_asn_operator(enum Lexicon token) {
    return (
        token == EQUAL
        || token == MINUSEQ
        || token == PLUSEQ
        || token == BOREQL
        || token == BANDEQL
        || token == BNOTEQL
    );
}

/*
   returns bool if token is a binary operator
*/
bool is_operator(enum Lexicon token) {
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
        || token == PIPEOP
    );
}

/* null delimitated */
uint8_t eq_any_tok(enum Lexicon cmp, enum Lexicon buffer[]) {
  for (uint16_t i ;; i++)
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
    static enum Lexicon keywords[10] = {
        // STATIC, CONST,
        RETURN,
        // EXTERN, AS,
        IF, ELSE, FUNC_DEF, IMPORT,
        // IMPL,
        0
    };    
    return eq_any_tok(token, keywords);
}

bool is_num_negative(const char * source, struct Token *token) {
    return token->type == INTEGER && *(source + token->start) == '-';
}

bool is_unit(enum Lexicon tok)
{
    
  return \
    tok == STRING_LITERAL 
    || tok == INTEGER
    || tok == WORD;
}

bool is_group(enum Lexicon tok)
{
  return \
    tok == TupleGroup 
    ||tok == ListGroup
    ||tok == IndexGroup
    ||tok == MapGroup  
    ||tok == CodeBlock;
}

bool is_group_modifier(enum Lexicon tok) {
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
        || tok == STRUCT;
}
