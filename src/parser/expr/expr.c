#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "../../prelude.h"
#include "../lexer/lexer.h"
#include "../lexer/helpers.h"

#include "expr.h"


#define WORKING_BUF_SZ 128
#define QUEUE_BUF_SZ 256
#define OPERATOR_BUF_SZ 64
#define END_PRECEDENCE 127

enum Associativity {
    NONASSOC,
    RASSOC,
    LASSOC
};

enum Associativity get_assoc(enum Lexicon token) {
    switch(token) {
        case POW:
            return RASSOC;
        default:
            return LASSOC;
    }
}

/*
    precendense table:
      ") ] }"   : 127 non-assoc
      "."     : 126 L
      
    
      "^"       : 6 R (1 ^ 2 ^ 3) -> (1 ^ (2 ^ 3))
      "/ * %"   : 5 L  (4 / 2 * 2) -> ((4 / 2) * 2)
      "+ -"     : 4 L
      "!= == >= > <= < && ||": 3 L
      "!"       : 2 L
      "( [ {"   : 0 non-assoc
*/
int8_t op_precedence(enum Lexicon token) {
    if (is_close_brace(token))
        return END_PRECEDENCE;
    
    if (token == DOT)
        return 126;
    
    else if (token == POW)
        return 6;

    else if (token == MUL 
        || token == DIV
        || token == MOD)
        return 5;

    else if (token == ADD
        || token == SUB)
        return 4;
    
    else if (token == ISEQL
        || token == ISNEQL
        || token == GTEQ
        || token == LTEQ
        || token == GT
        || token == LT
        || token == AND
        || token == OR)
        return 2;
    
    else if (is_open_brace(token))
        return 0;
    
    return -1;
}

/*
    Search stack for the last token type.
*/
int8_t get_last_insert_of_ty_tok(
    struct Token *operators[],
    usize operator_sz,
    enum Lexicon type,
    struct Token *out,
    usize *out_idx
) {

    struct Token *head;
    
    for (usize i=0; operator_sz > i; i++)
    {
        head = operators[operator_sz - i];   
        if (type == head->type) {
            out = head;
            *out_idx = operator_sz - i;
            return 1;
        }
    } 
    
    return 0;
}



struct Token * new_token(struct TokenHints *bookkeeping, enum Lexicon token, usize start, usize end) {
    bookkeeping->token_pool[bookkeeping->pool_i].type = token;
    bookkeeping->token_pool[bookkeeping->pool_i].start = start;
    bookkeeping->token_pool[bookkeeping->pool_i].end = end;
    
    bookkeeping->pool_i += 1;

    return &bookkeeping->token_pool[bookkeeping->pool_i];
}


/*
  Shunting yard expression parsing algorthim 
  https://en.wikipedia.org/wiki/Shunting-yard_algorithm
  --------------
*/

#define STACK_SZ 128
#define _OP_SZ 128

                                

/* if bit set, expects operand to be the next token */
#define EXPECTING_OPERAND        1

/* if bit set, expects binary operator to be the next token */
#define EXPECTING_OPERATOR       2

/* if bit set, expects opening brace type be the next token */
#define EXPECTING_OPEN_BRACE     4    

/* if bit set, expects closing brace type be the next token */
#define EXPECTING_CLOSE_BRACE    8

/* if bit set, expects a comma be the next token */
#define EXPECTING_COMMA          16   

/* if bit set, expects a colon until bracket_brace token type is closed */
#define EXPECTING_COLON          32 

/* if bit set, expects a token to follow */
#define EXPECTING_NEXT           64  

/* reserved */
#define ___XXXXXXXXXXXXXX        128 


/*
    check flag, and if present, unset it.
*/
int8_t check_flag(uint8_t *set, uint8_t flag){
    return (*set) & flag;
}

int8_t postfix_expr(
    struct Token *tokens[],
    usize expr_size,
    
    struct Token *output[],
    usize output_sz,
    usize *output_ctr,
    
    struct TokenHints *bookkeeping,
    struct CompileTimeError *err
){
    /* operator stack */
    struct Token *operators[STACK_SZ];    
    /*
        operators stack pointer, always points 
        to the next available index
    */
    int8_t operators_ctr = 0;
  
    /* head of the operator stack*/
    struct Token *head = NULL;
    
    /* head of the operator stacks precedence*/
    int8_t head_precedense = 0;

    /* references to tokens */
    struct Token *index_hints[STACK_SZ];
    uint8_t index_hint_ctr = 0;

    /* current tokens precedence */
    int8_t precedense = 0;

    /*
        The grouping stack is used to track the amount 
        of sub-expressions inside an expression. (See lexer.h)
        We generate GROUPING tokens based on the stack model
        our parser uses.

        For every new brace token-type added into the operator-stack
        increment the grouping stack, and initalize it to 0.
        For every comma, increment the current grouping-stack's head by 1.

        Once the closing brace is found;
        if this stack's head is larger than 0,
        we have a set/grouping of expressions. 
    */
    uint8_t grouping_stack[STACK_SZ];
    uint8_t grouping_stack_ctr = 0;

    /*
        Indexable_Array[START:END:SKIP]
    */
    uint8_t index_colon_stack[STACK_SZ];
    uint8_t index_colon_stack_ctr = 0;
    /* used for stack digging */
    usize _last_insert = 0;

    /*
        this is a stack array,
        where each set/grouping has 
        the number of its atomics accounted for here
    */
    uint16_t atomics_stack[STACK_SZ];
    uint8_t atomics_ctr = 0;

    
    bookkeeping->flags = 0 | EXPECTING_OPERAND 
        | EXPECTING_OPEN_BRACE
        | EXPECTING_NEXT;

    for (usize i = 0; expr_size > i; i++)
    {
        /*
            if the token is data (value/variable in the source code),
            place it directly into our stack
        */
        if (*output_ctr > output_sz
            || operators_ctr > (int8_t)_OP_SZ )
            return -1;
        
        else if (is_data(tokens[i]->type))
        {
            if (!check_flag(&bookkeeping->flags, EXPECTING_OPERAND))
                return -1;

            if (!check_flag(&bookkeeping->flags, EXPECTING_COLON))
                bookkeeping->flags = 0 | EXPECTING_OPERATOR 
                    | EXPECTING_OPEN_BRACE
                    | EXPECTING_CLOSE_BRACE
                    | EXPECTING_COMMA
                    | EXPECTING_COLON;
            else 
                bookkeeping->flags = 0 | EXPECTING_OPERATOR 
                    | EXPECTING_OPEN_BRACE
                    | EXPECTING_CLOSE_BRACE
                    | EXPECTING_COMMA;
            
            /*
                peek ahead to see if this 
                could possibly be a function call.
            */
            if (tokens[i]->type == WORD 
                && tokens[i+1]->type == PARAM_OPEN)
            {
                /* expecting open brace */
                bookkeeping->flags = 0 | EXPECTING_OPEN_BRACE | EXPECTING_NEXT;
                
                /* overflow check */
                if (bookkeeping->function_hints_ctr > bookkeeping->function_hints_sz)
                    return -1;
                
                /* 
                    book-keeping to remember functions
                */
                bookkeeping->function_hints[bookkeeping->function_hints_ctr] = tokens[i];
                bookkeeping->function_hints_ctr += 1;

                
                /* place func name into operator stack */
                operators[operators_ctr] = tokens[i];
                operators_ctr += 1;
            }

            /*
            *    peek ahead to see if this 
            *    could possibly be an index-access.
            */
            else if (tokens[i]->type == WORD && tokens[i+1]->type == BRACKET_OPEN)
            {
                /* expecting operand = 1 */
                bookkeeping->flags = 0 | EXPECTING_OPEN_BRACE | EXPECTING_NEXT;
                
                if (index_hint_ctr > STACK_SZ)
                    return -1;
                
                index_hints[index_hint_ctr] = tokens[i+1];
                index_hint_ctr += 1;
            }

            /*
            *   if not a function, place it into the
            *   output as an operand 
            */
            else
            {
                output[*output_ctr] = tokens[i];
                *output_ctr += 1;
            }

            /* increment atomics counter */
            atomics_stack[atomics_ctr] += 1;
            continue;
        }

        else if (is_bin_operator(tokens[i]->type))
        {
            if (!check_flag(&bookkeeping->flags, EXPECTING_OPERATOR))
                return -1;

            bookkeeping->flags = 0 | EXPECTING_OPERAND | EXPECTING_OPEN_BRACE | EXPECTING_NEXT;

            precedense = op_precedence(tokens[i]->type);

            /* unrecongized token */
            if (precedense == -1)
                return -1;

            /*
                no operators in operators-stack, 
                so no extra checks needed
            */
            else if (operators_ctr == 0) { 
                operators[0] = tokens[i];
                operators_ctr += 1;
                continue;
            }

            /* Grab the head of the operators-stack */
            head = operators[operators_ctr-1];

            /*
                if the head of the operator stack is an open brace
                we don't need to do anymore checks
                before placing the operator
            */
            if (is_open_brace(head->type)) {
               
                operators[operators_ctr] = tokens[i];
                operators_ctr += 1;
                continue;
            }
            
            /*
                while `head` has higher precedence 
                than our current token pop operators from
                the operator-stack into the output
            */
            while(op_precedence(head->type) >= precedense && operators_ctr > 0)
            {
                head_precedense = op_precedence(head->type);

                if (is_open_brace(head->type))
                    break;
                
                /*
                    pop operators off the stack
                    into the output
                */
                if (head_precedense > precedense)
                {
                    if (*output_ctr > output_sz)
                        return -1;
                    
                    output[*output_ctr] = head;
                    *output_ctr += 1;
                }

                /* 
                    If left associated, push equal
                    precedence operators onto the output 
                */
                else if (precedense == head_precedense)
                {
                    if (get_assoc(tokens[i]->type) == LASSOC)
                    {
                        if (*output_ctr > output_sz)
                            return -1;
                        output[*output_ctr] = head;
                        *output_ctr += 1;
                    }
                }

                /* discard operator after placed in output */
                operators_ctr -= 1;
                if (operators_ctr <= 0)
                    break;
                
                head = operators[operators_ctr-1];
            }

            /* place low precedence operator */            
            operators[operators_ctr] = tokens[i];
            operators_ctr += 1;
        }

        else if (is_close_brace(tokens[i]->type))
        {
            if (!check_flag(&bookkeeping->flags, EXPECTING_CLOSE_BRACE))
                return -1;

            bookkeeping->flags = 0 | EXPECTING_OPEN_BRACE 
                | EXPECTING_CLOSE_BRACE
                | EXPECTING_COMMA
                | EXPECTING_OPERATOR;
            
            /* Operators stack is empty */
            if (operators_ctr == 0)
                return -1;
            
            /*
                The current token is the inverted brace 
                type of the top of operator-stack.
            */
            else if (operators[operators_ctr - 1]->type == invert_brace_tok_ty(tokens[i]->type)) {
                /* 
                    if there is no atomics,
                    then we know this is an empty nesting (set of 0 members/expressions)
                    Empty nestings may be symbolic for functionality,
                    so include a GROUP(0) token in the output.
                */
                if (atomics_stack[atomics_ctr] == 0) {
                    /* overflow check */
                    if (bookkeeping->set_ctr > bookkeeping->sets_sz
                        || grouping_stack_ctr > STACK_SZ)
                            return -1;

                    /* Create GROUP(0) token, and reference in sets */
                    bookkeeping->sets[bookkeeping->set_ctr] = new_token(bookkeeping, GROUPING, tokens[i]->start, 0);
                    
                    /* add it to output */
                    output[*output_ctr] = bookkeeping->sets[grouping_stack_ctr];
                    
                    *output_ctr += 1;
                    bookkeeping->set_ctr += 1;
                }

                /* discard opening brace token */
                operators_ctr -= 1;
                continue;
            }

            /* should be atleast one operator in the stack */
            else if (operators_ctr <= 0)
                return -1;

            /* pop operators off of the operator-stack into the output */
            while(operators_ctr > 0) {
                /* Grab the head of the stack */
                head = operators[operators_ctr-1];

                /* ends if tokens inverted brace is found*/
                if (head->type == invert_brace_tok_ty(tokens[i]->type)) {
                    /* do not discard opening brace yet
                    // operators_ctr -= 1;
                    */
                    break;
                }
                /* otherwise pop into output */
                else {
                    if (*output_ctr > output_sz)
                        return -1;
                    
                    output[*output_ctr] = head;
                    *output_ctr += 1;
                    operators_ctr -= 1;
                }
            }

            /*
            //  If enough hints are made,
            //  we can safely assume this is
            //  an index-access.
            */
            if(grouping_stack[grouping_stack_ctr] == 0 
                && head->type == BRACKET_OPEN
                && index_hint_ctr > 0
                && index_hints[index_hint_ctr-1] == head)
            {    

                /* 
                    * TODO * DOES NOT ACCOUNT FOR ATOMICS 
                    
                    Should be able to parse Foo[::1]
                    IT DOES NOT
                */

                /* there shouldn't be more than 2 colons */
                if (index_colon_stack[index_colon_stack_ctr] > 2
                    /* overflow check */
                    || *output_ctr+3-index_colon_stack[index_colon_stack_ctr] > output_sz)
                    return -1;
                
                /* 
                    for every argument thats missing from A[N:N:N]
                    fill in N as NULLTOKEN
                */
                
                for (uint8_t i=index_colon_stack[index_colon_stack_ctr]; 2 > i; i++) {
                    output[*output_ctr] = new_token(bookkeeping, NULLTOKEN, 0, 0);
                    *output_ctr += 1;
                }

                output[*output_ctr] = new_token(bookkeeping, INDEX_ACCESS, 0, 0);
                *output_ctr += 1;
            }
            else
            {
                /* 
                    place a grouping token after 
                    all the appriotate operations
                    have been placed. 
                */
                if (bookkeeping->set_ctr > bookkeeping->sets_sz
                    || grouping_stack_ctr > STACK_SZ)
                        return -1;
                /* 
                    setup token from allocated pool
                    and build a grouping operator
                */            
                /* using `start` as a position cordinate to determine the brace type */
                /* using `end` to signify how many values to pull from the stack */
                /* reference new token from pool into groups and output*/
                bookkeeping->sets[bookkeeping->set_ctr] = new_token(
                    bookkeeping,
                    GROUPING,
                    head->start,
                    grouping_stack[grouping_stack_ctr] + 1
                );
                bookkeeping->set_ctr += 1;
                
                /* place grouping operator on the output */
                output[*output_ctr] = bookkeeping->sets[grouping_stack_ctr];
                *output_ctr += 1;
            }

            /* discard opening brace from operator stack */
            operators_ctr -= 1;
            
            /* discard the groups sub-expression count. */
            grouping_stack_ctr -= 1;
            
            /* discard atomics count */
            atomics_ctr -= 1;

            /* 
                if the top of the operator-stack is a function,
                push it onto the output, and discard
                from the operator stack
            */
            if (operators_ctr > 0
                && operators[operators_ctr-1]->type == WORD)
            {    
                if (*output_ctr > output_sz)
                    return -1;
                
                output[*output_ctr] = head;
                *output_ctr += 1;
                operators_ctr -= 1;
            }
        }

        /*
            place opening brace on the operator stack,
            and update book-keeping stacks
        */
        else if (is_open_brace(tokens[i]->type))
        {
            if (check_flag(&bookkeeping->flags, EXPECTING_OPEN_BRACE))
                return -1;
            
            bookkeeping->flags = 0 | EXPECTING_OPERAND 
                | EXPECTING_CLOSE_BRACE
                | EXPECTING_OPEN_BRACE
                | EXPECTING_NEXT;
            
            if (tokens[i]->type == BRACKET_OPEN)
                bookkeeping->flags = bookkeeping->flags | EXPECTING_COLON;

            /* overflow check */
            if (grouping_stack_ctr > STACK_SZ 
                || index_colon_stack_ctr > STACK_SZ
                || atomics_ctr > STACK_SZ
                || operators_ctr > (int8_t)_OP_SZ)
                return -1;

            /* increment grouping/sets stack counter*/
            grouping_stack_ctr += 1;
            grouping_stack[grouping_stack_ctr] = 0;
             
            /* increment colon stack counter*/
            index_colon_stack_ctr += 1;
            index_colon_stack[index_colon_stack_ctr] = 0;

            /* increment colon stack counter*/
            atomics_ctr += 1;
            atomics_stack[atomics_ctr] = 0;
            
            /* place opening brace into operators */
            operators[operators_ctr] = tokens[i];
            operators_ctr += 1;
        }

         /*
            only index accesses have `:` in them
            so here we can hint that this is most
            likely an index access
        */
        else if (tokens[i]->type == COLON)
        {
            if (!check_flag(&bookkeeping->flags, EXPECTING_COLON) 
                || index_colon_stack_ctr > STACK_SZ
                || index_colon_stack[index_colon_stack_ctr] > 2)
                return -1;

            bookkeeping->flags = 0 | EXPECTING_OPERAND 
                | EXPECTING_OPEN_BRACE
                | EXPECTING_NEXT;

            /*
                This is a bit hacky,
                but since any opening brace increments this.
                we can use it to track the amount of atomics 
                in each segment of `A[N:N:N]` where it individually 
                traces each N.
                
                Respectively we can do this without 
                much hastle because index-access 
                start with the brace token '['

                Will be its own stack frame `[` to `:`,  
                then from `:` to `:` will be its own stack frame.
                Finally `:` to `]` will be its own stack frame, respectively.
            */
            atomics_ctr += 1;

            /* 
                check that there is a `[` 
                token in the operator-stack
            */ 
            if (get_last_insert_of_ty_tok(
                operators,
                operators_ctr,
                BRACKET_OPEN,
                head,
                &_last_insert) == 1
            ){
                index_colon_stack[index_colon_stack_ctr] += 1;
                index_hints[index_hint_ctr] = head;
                index_hint_ctr += 1;
            }
            else 
                return -1;
        }
        /*
            Increment our current grouping-set
        */
        else if (tokens[i]->type == COMMA) {
            if (!check_flag(&bookkeeping->flags, EXPECTING_COMMA)
                || grouping_stack_ctr > STACK_SZ)
                return -1;
            
            bookkeeping->flags = 0 | EXPECTING_OPERAND 
                | EXPECTING_OPEN_BRACE 
                | EXPECTING_NEXT;

            grouping_stack[grouping_stack_ctr] += 1;
        }
        else
        {
            #ifdef DEBUG 
                printf("debug: token fell through precedense [%s]\n", ptoken(tokens[i]->type));
            #endif
        }
    }

    /*
        dump the remaining operators onto the output
    */
    while(operators_ctr > 0)
    {
        /*
            any remaining params/brackets/braces are unclosed
            indiciate invalid expressions    
        */
        if (is_open_brace(operators[operators_ctr-1]->type) == END_PRECEDENCE)
            return -1;
        
        if (*output_ctr > output_sz)
            return -1;
        
        output[*output_ctr] = operators[operators_ctr-1];
        
        *output_ctr += 1;
        operators_ctr -= 1;
    }

    return 0;
}


int8_t construct_expr_ast(char *line, struct Token tokens[], usize ntokens, struct Expr *expr) {
    // struct Token *masked_tokens[512];
    // struct Token *queue[512];
    // struct Token masks[32];
    // struct Token *working_buf[128];
    // size_t masks_ctr = 0;
    // size_t masked_tokens_ctr = 0;
    // size_t queue_ctr = 0;

    // if (!is_balanced(tokens, ntokens))
    //     return -1;
    
    // if (mk_fnmask_tokens(
    //     masked_tokens, 512, &masked_tokens_ctr,
    //     tokens, ntokens,
    //     masks, 32, &masks_ctr) == -1)
    //     return -1;
    

    // if (tokens[0].type == NOT) {}
    // else if (is_fncall(tokens)) {}
    // else if (is_data(tokens[0].type)) {}
    // else if (tokens[0].type == DOT) {}

    // if (mk_fnmask_tokens(
    //     masked_tokens, 512, &masked_tokens_ctr,
    //     tokens, ntokens,
    //     masks, 32, &masks_ctr) == -1)
    //     return -1;
    
    // /*
    //     reorders `*output[]` according to PEMDAS
    //     into the buffer `*queue[]`
    // */
    // if (construct_postfix_queue(masked_tokens, masked_tokens_ctr, queue, 512, &queue_ctr) == -1)
    //     return -1;
    
    // for (int i=0; queue_ctr > i; i++) {
    //     else if (is_data(queue[i])) {}
    //     if (is_bin_operator(queue[i]) {}
    // }

    /*todo: create expr tree*/
    return 0;
}
