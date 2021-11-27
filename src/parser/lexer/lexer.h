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
    usize start;
    usize end;
    enum Lexicon type;
};

int8_t tokenize(
    const char *line,
    struct Token tokens[],
    usize *token_ctr,
    usize token_sz,
    struct CompileTimeError *error
);

#endif
