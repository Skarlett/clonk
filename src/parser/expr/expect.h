#include <stdint.h>
#include <sys/types.h>
#include "../lexer/lexer.h"
#include "expr.h"

FLAG_T expecting_next(enum Lexicon tok);
int8_t is_token_unexpected(struct Token *current, struct Group *ghead, FLAG_T expecting);

#define EXPECTING_SYMBOL         2
#define EXPECTING_INTEGER        4
#define EXPECTING_STRING         8
#define EXPECTING_ARITHMETIC_OP 16

/* . */
#define EXPECTING_APPLY_OPERATOR 32
#define EXPECTING_OPEN_BRACKET   64   
#define EXPECTING_CLOSE_BRACKET  128
#define EXPECTING_OPEN_PARAM     256    
#define EXPECTING_CLOSE_PARAM    512
#define EXPECTING_OPEN_BRACE     1024
#define EXPECTING_CLOSE_BRACE    2048

#define EXPECTING_NEXT           4096 
/* : , */
#define EXPECTING_DELIMITER      8192
