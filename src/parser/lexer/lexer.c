#include "lexer.h"
#include "../../prelude.h"

#include <stdio.h>
#include <string.h>

enum Lexicon tokenize_char(char c) {
    switch (c) {
        case ' ': return WHITESPACE;
        case '\n': return NEWLINE;
        case '\t': return WHITESPACE;
        case '\r': return WHITESPACE;
        case '/':  return DIV;
        case '=':  return EQUAL;
        case '"':  return QUOTE;
        case '{':  return BRACE_OPEN;
        case '}':  return BRACE_CLOSE;
        case '(':  return PARAM_OPEN;
        case ')':  return PARAM_CLOSE;
        case '[':  return BRACKET_OPEN;
        case ']':  return BRACKET_CLOSE;
        case ';':  return SEMICOLON;
        case ',':  return COMMA;
        case '+':  return ADD;
        case '-':  return SUB;
        case '*':  return MUL;
        case '^':  return POW;
        case '>':  return GT;
        case '<':  return LT;
        case '&':  return AMPER;
        case '|':  return PIPE;
        case '%':  return MOD;
        case '_':  return UNDERSCORE;
        case ':':  return COLON;
        case '!':  return NOT;
        case '#':  return POUND;
        case '@':  return ATSYM;
        default :  break;
    }

    for (int d_i = 0; sizeof(DIGITS) > d_i; d_i++) {
        if (c == DIGITS[d_i])
            return DIGIT;
    }

    for (int a_i = 0; sizeof(ALPHABET) > a_i; a_i++) {
        if (c == ALPHABET[a_i])
            return CHAR;
    }

    
    return UNKNOWN;
}

int is_operator_complex(enum Lexicon compound_token) {
    return (
        compound_token == ISEQL
        || compound_token == ISNEQL 
        || compound_token == GTEQ 
        || compound_token == LTEQ
        || compound_token == AND
        || compound_token == OR
        || compound_token == MINUSEQ
        || compound_token == PLUSEQ
        || compound_token == GT
        || compound_token == LT
    );
}

int can_upgrade_token(enum Lexicon token) {
    return (
        token == DIGIT
        || token == CHAR
        || token == UNDERSCORE
        || token == QUOTE
        || token == EQUAL
        || token == NOT
        || token == GT
        || token == LT
        || token == ADD
        || token == SUB
        || token == AMPER
        || token == PIPE
        || token == POUND
    );
}

int set_compound_token(enum Lexicon *compound_token, enum Lexicon token) {
    switch (token) {
        case DIGIT: 
            *compound_token = INTEGER;
            break;
        
        case NOT: 
            *compound_token = ISNEQL;
            break;

        case CHAR:
            *compound_token = WORD;
            break;
        
        case UNDERSCORE:
            *compound_token = WORD;
            break;

        case QUOTE:
            *compound_token = STRING_LITERAL;
            break;

        case EQUAL:
            *compound_token = ISEQL;
            break;

        case GT:
            *compound_token = GTEQ;
            break;
        
        case LT:
            *compound_token = LTEQ;
            break;
        
        case ADD:
            *compound_token = PLUSEQ;
            break;
        
        case SUB:
            *compound_token = MINUSEQ;
            break;
        
        case AMPER:
            *compound_token = AND;
            break;
        
         case POUND:
            *compound_token = COMMENT;
            break;

        case PIPE:
            *compound_token = OR;
            break;

        default: return -1;
    }
    
    return 0;
}

/*
 Returns a boolean if compound_token should continue consuming
 tokens
*/
int continue_compound_token(enum Lexicon token, enum Lexicon compound_token, u64 span_size) {
    return (
        // Integer
        (compound_token == INTEGER && token == DIGIT) 
        
        // Variables
        || (compound_token == WORD && token == CHAR)
        || (compound_token == WORD && token == UNDERSCORE)
        || (compound_token == WORD && token == DIGIT)
        // ---
        
        // String literal
        // "..."
        || (compound_token == STRING_LITERAL && token != QUOTE)
        // operators 
        // !=
        || (compound_token == ISNEQL && token == EQUAL && 1 > span_size)
            // ==
        || (compound_token == ISEQL && token == EQUAL && 1 > span_size)
            // >=
        || (compound_token == GTEQ && token == EQUAL && 1 > span_size)
            // <=
        || (compound_token == LTEQ && token == EQUAL && 1 > span_size)
            // &&
        || (compound_token == AND && token == AMPER && 1 > span_size)
            // ||
        || (compound_token == OR && token == PIPE && 1 > span_size)
            // +=
        || (compound_token == PLUSEQ && token == EQUAL && 1 > span_size)
            // -=
        || (compound_token == MINUSEQ && token == EQUAL && 1 > span_size)
            // # ... \n
        || (compound_token == COMMENT && token != NEWLINE)
    );
}


enum Lexicon invert_operator_token(enum Lexicon compound_token) {
    switch (compound_token) {
        case COMMENT: return POUND;
        case ISEQL: return EQUAL;
        case GTEQ: return GT;
        case LTEQ: return LT;
        case AND: return AMPER;
        case OR: return PIPE;
        case MINUSEQ: return SUB;
        case PLUSEQ: return ADD;
        case ISNEQL: return NOT;
        default: return UNKNOWN;
    }
}

int derive_keyword(char * line, struct Token *t) {
    static enum Lexicon lexicon[] = {
        STATIC, CONST, RETURN, EXTERN, 
        AS, IF, ELSE, FUNC_DEF, IMPORT, IMPL,
        AND, OR
    };

    static char *keywords[] = {
        "static", "const", "return", "extern",
        "as", "if", "else", "def", "import", "impl",
        "and", "or", 0
    };

    for (int i=0; 12 > i; i++){
        /*token.end+1 since the fields naturally are indexable*/
        if (strlen(keywords[i]) == ((t->end+1) - t->start) 
            && strncmp(line + t->start, keywords[i], t->end - t->start) == 0)
        {
            t->token = lexicon[i];
            return 1;
        }
    }

    return 0;
}

int finalize_compound_token(struct Token *token, char * line, enum Lexicon lexed) {
    if (token->token == STRING_LITERAL) {
        /*
          Error: string is missing a ending quote
        */
        if (lexed != QUOTE)
            return -1;
        
        token->end += 1;
    }
    
    else if (token->token == WORD)
        derive_keyword(line, token);

    /*
    / check if its an operator, and that its lenth is 2
    / if not - down grade the operator from its complex version
    */
    else if (is_operator_complex(token->token) && token->start - token->end == 0) {
        token->token = invert_operator_token(token->token);
        
        /* Error: unknown/null token when inverted*/
        if (token->token == UNKNOWN || token->token == NULLTOKEN)
            return -1; 
    }

    return 0;
}

int tokenize(char *line,  struct Token tokens[], usize token_idx) {
    enum Lexicon lexed = NULLTOKEN, 
        compound_token = NULLTOKEN;
    struct Token token;
    
    uint8_t operator_ctr = 0;

    u64 start_at = 0;
    u64 span_size = 0;
    int new_tokens = 0;

    for (u64 i=0; strlen(line) > i; i++)
    {
        if (line[i] == 0) continue;
        else if (is_utf(line[i])) return -1;

        lexed = tokenize_char(line[i]);
        
        if (compound_token == NULLTOKEN && can_upgrade_token(lexed))
        {
            set_compound_token(&compound_token, lexed); 
            start_at = i;
            span_size = 0;
            continue;
        }

        /* continuation of complex token */
        else if (continue_compound_token(lexed, compound_token, span_size)) {
            span_size += 1;
            continue;
        }

        else if (compound_token != NULLTOKEN)
        {
            /* completion of complex token */
            if (compound_token == COMMENT) {
                compound_token = NULLTOKEN;
                lexed = NEWLINE;
                continue;
            }

            token.start = start_at;
            token.end = start_at+span_size;
            token.token = compound_token;
            
            if (finalize_compound_token(&token, line, lexed) == -1)
                return -1;

            tokens[token_idx+new_tokens] = token;
            
            new_tokens += 1;
            compound_token = NULLTOKEN;
            
            if (token.token == STRING_LITERAL) {
                token.end += 1;
                continue;
            }
        }
        
        if (can_upgrade_token(lexed)) {
            i--;
            continue;
        }
        /*
            this fires for non-compound tokens,
            this 'if statement' isn't inside an 'else-block', 
            because if a compound token is compleed, 
            the current token being lexed still needs to be added.
        */
        else if (lexed != WHITESPACE && lexed != NEWLINE && lexed != NULLTOKEN) 
        {
            struct Token token = {
                .start = i,
                .end = i,
                .token = lexed
            };

            compound_token = NULLTOKEN;
            start_at = 0;
            tokens[token_idx+new_tokens] = token;
            new_tokens += 1;
        }
    }
    
    /*
       the source code (char *line) sometimes 
       doesn't run into a condition to break a complex token's continuation,
       so the for loop ends, before the complex token is stored.
    */
    if (compound_token != NULLTOKEN && compound_token != COMMENT)
    {
        struct Token token = {
            .start = start_at,
            .end = start_at+span_size,
            .token = compound_token
        };

        if (finalize_compound_token(&token, line, lexed) == -1)
            return -1;

        tokens[token_idx+new_tokens] = token;
        new_tokens += 1;
    }

    return new_tokens;
}
