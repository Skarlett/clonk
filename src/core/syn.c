

//synthesizer
// maintains sanity by constructing some rules
// in which the interpreter expects

#include <stdlib.h>
#include "lexer.h"
#include "expr.h"
#include "ast.h"


void synthesize(BlockStatement *block) {
    for (size_t i=0; block->length > i; i++) {
        
    }
}


// `if` + `block`

// `define` + `block`

// `expr` + `operator` + `expr`

// `define` + `block` + `return`

// ! `block` + `return`

// `int` `(* | + | - )`= `int`

// `int` `(* | + | - | ^ | % | / | `|` | & )` `int`

// `string` += `string`

