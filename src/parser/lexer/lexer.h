#ifndef _HEADER__LEXER__
#define _HEADER__LEXER__

#include <stdlib.h>
#include <stdint.h>
#include "../../prelude.h"
#include "../error.h"

#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"

enum Lexicon {
    UNDEFINED,
    
    // End of file token
    EOFT,
    
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

    // >
    GT,

    // <
    LT,

    // >=
    GTEQ,

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
    
    // &
    AMPER,

    // |
    PIPE,

    // &&
    AND,

    // ||
    OR,

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

    // 0-9
    DIGIT,
    
    // ,
    COMMA,
    //********* START OF COMPLEX TOKENS ********
    // Complex tokens wont show up in the first round of lexer'ing
    // they're generated from combinations of tokens
    // "fn"
    //********* START OF COMPLEX LEXICONS ********

    // [NUM, ..] WHITESPACE|SEMICOLON   
    // 20392
    INTEGER,

    // [CHARACTER, ..] WHITESPACE|SEMICOLON
    // something
    WORD,

    // [QUOTE, ... QUOTE]
    // something
    STRING_LITERAL,

    // ()
    //EMPTY_PARAM,

    // []
    //EMPTY_BRACKET,

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
        and should never been seen in the token stream
    */

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

    GROUPING,

    /*

    INDEX_ACCESS acts as a function in the postfix representation
    that takes 4 arugments off the stack
    'source, start, end, skip' in that order.
    
    when INDEX_ACCESS arguments maybe padded with NULLTOKENS
    inserted by the first stage or the user.
    NULLTOKENS when parsed into expression trees 
    will assume their value based on position except
    for the first argument (the source being index).
    The 2nd argument (start) will assume 0 if NULLTOKEN is present.
    The 3rd argument (end) will assume the length of the array if NULLTOKEN is present.
    The 4th (skip) will assume 1 if NULLTOKEN is present.

    Examples:
      token output: WORD   INTEGER INTEGER INTEGER INDEX_ACCESS 
                    source start   end     skip    operator
              text: foo[1::2]
        postfix-IR: foo 1 NULL 2 INDEX_ACCESS
    */
    INDEX_ACCESS,

    // foo(a)(b)
    // foo(a) -> func(b) -> T
    // foo(a)(b) -> ((a foo), b G(2)) DyCall
    // foo(a)(b)(c) -> a foo b G(2) DyCall c G(2) DyCall
    APPLY

    // soon to be map operator?
    // MAP
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

    In the case of `type` being of FNMASK or COMPOSITE:
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
    char *line,
    struct Token tokens[],
    usize *token_ctr,
    usize token_sz,
    struct CompileTimeError *error
);

#endif
