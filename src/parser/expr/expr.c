#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "../../prelude.h"
#include "../lexer/lexer.h"
#include "../lexer/helpers.h"
#include "../lexer/debug.h"

#include "expr.h"
#include "pool.h"
#include "helpers.h"

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

int8_t unwind_params_needed(struct Token *tok, usize *ptr_sz) {
    if (tok->type == GROUPING)
        *ptr_sz = tok->end;
    
    if (tok->type == INDEX_ACCESS)
        *ptr_sz = 5;
    
    else if (is_bin_operator(tok->type))
        *ptr_sz = 2;

    // next token is a group
    else if (tok->type == WORD)
        *ptr_sz = 0;

    else
        return -1;

    return 0;
}


/*
    Search stack for the last token type.
*/
int8_t get_last_inserted_token(
    struct Token *operators[],
    usize operator_sz,
    enum Lexicon match[],
    enum Lexicon match_len,
    struct Token *out,
    usize *out_idx
) {

    usize i=0, j=0;
    struct Token *head;

    if (0 >= match_len)
        return 0;
    
    for (i=0; operator_sz > i; i++)
    {
        head = operators[operator_sz - i];   
        for (j=0; match_len > j; j++) {
            if (match[j] == head->type) {
                out = head;
                *out_idx = operator_sz - i;
                return 1;
            }
        }
    } 
    
    return 0;
}


struct Token * new_token(struct ExprParserState *bookkeeping, enum Lexicon token, usize start, usize end) {
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
/*
    check flag, and if present, unset it.
*/
int8_t check_flag(FLAG_T set, FLAG_T flag){
    return set & flag;
}

void set_flag(FLAG_T *set, FLAG_T flag){
    *set = *set | flag;
}

void unset_flag(FLAG_T *set, FLAG_T flag){
    *set = *set & ~flag;
}

int8_t postfix_expr(
    struct Token *tokens[],
    usize expr_size,
    
    struct Token *output[],
    usize output_sz,
    usize *output_ctr,
    
    struct ExprParserState *state,
    struct CompileTimeError *err
){
    /* operator stack */
    struct Token *operators[STACK_SZ];
    /*
        operators stack pointer, always points 
        to the next available index
    */
    int16_t operators_ctr = 0;
  
    /* head of the operator stack*/
    struct Token *head = NULL;
    
    /* head of the operator stacks precedence*/
    int8_t head_precedense = 0;

    /* used for stack digging */
    usize last_insert = 0;

    /* track the amount of elements to remove */
    usize unwind_param_ctr = 0;
    usize unwind_param_total = 0;

    enum Lexicon search_token_buffer[4];

    /* references to tokens */
    /* TODO */
    //struct Token *index_hints[STACK_SZ];
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

    
    /*Indexable_Array[START:END:SKIP]*/
    uint8_t index_colon_stack[STACK_SZ];
    uint8_t index_colon_stack_ctr = 0;
    

    /*
        this is a stack array,
        where each set/grouping has 
        the number of its atomics accounted for here
    */
    uint16_t atomics_stack[STACK_SZ];
    uint8_t atomics_ctr = 0;
    
    set_flag(&state->flags, EXPECTING_OPERAND 
        | EXPECTING_OPEN_BRACE
        | EXPECTING_NEXT);
    
    for (usize i = 0; expr_size > i; i++)
    {

        /*
            If the PANIC_FLAG is set, we'll attempt to recover the parser
            from its failing state. We'll do this by discarding
            the entire expression within its error space.
        */
        if (!check_flag(state->flags, PANIC_FLAG)) {
            set_flag(&state->flags, INCOMPLETE_FLAG);

            search_token_buffer[0] = BRACKET_OPEN;
            search_token_buffer[1] = PARAM_OPEN;
            /*
                search for BRACK/PARAM
                open tokens in the operator-stack
            */
            if (get_last_inserted_token(
                    operators,
                    operators_ctr,
                    search_token_buffer,
                    2,
                    head,
                    &last_insert) == 1)
            {
                /* unwind operators until brace type is found*/
                while (head != operators[operators_ctr - 1]){
                    unwind_params_needed(operators[operators_ctr - 1], &unwind_param_ctr);
                    unwind_param_total += unwind_param_ctr;
                    operators_ctr -= 1;
                }

                if(check_flag(state->flags, EXPECTING_NEXT) == 1)
                    unwind_param_total -= 1;
                    
                unwind_param_ctr = 0;

                for (int x=0; unwind_param_total > x; x++) {
                    if (*output_ctr > 0 && *output_ctr - x > 0) {
                            
                        if (unwind_param_total > *output_ctr)
                            return -1;
                            
                        if (unwind_params_needed(output[*output_ctr - x], &unwind_param_ctr) == 0){
                                
                            /* love, live, explode */
                            /*
                                grab the groups and placed operators from the output 
                            */
                            unwind_param_total += unwind_param_ctr;
                        }
                    }
                    else break;
                }
            }
            else {
                return -1;
            }
        }

        if (*output_ctr > output_sz
            || operators_ctr > (int8_t)_OP_SZ )
            return -1;
        
        /*
            if the token is data (value/variable in the source code),
            place it directly into our stack
        */
        else if (is_data(tokens[i]->type))
        {
            if (!check_flag(state->flags, EXPECTING_OPERAND))
                return -1;
            
            set_flag(&state->flags,
                EXPECTING_OPERATOR 
                    | EXPECTING_OPEN_BRACE
                    | EXPECTING_CLOSE_BRACE
                    | EXPECTING_COMMA
            );
            
            /*
                peek ahead to see if this 
                could possibly be a function call.
                When hinting functions, we place the WORD
                token ino
            */
            if (tokens[i]->type == WORD 
                && tokens[i+1]->type == PARAM_OPEN)
            {
                /* expecting open brace */
                state->flags = 0 | EXPECTING_OPEN_BRACE | EXPECTING_NEXT;
                
                /* overflow check */
                if (state->function_hints_ctr > state->function_hints_sz)
                    return -1;
                
                /* book-keeping to remember functions */
                state->function_hints[state->function_hints_ctr] = tokens[i];
                state->function_hints_ctr += 1;
                
                /* place func name into operator stack */
                operators[operators_ctr] = tokens[i];
                operators_ctr += 1;
            }

            /*
            *    peek ahead to see if this 
            *    could possibly be an index-access.
            *    we save the brace token as reference for later.
            */
            else if (tokens[i]->type == WORD && tokens[i+1]->type == BRACKET_OPEN)
            {
                /* expecting operand = 1 */
                state->flags = 0 | EXPECTING_OPEN_BRACE | EXPECTING_NEXT;
                
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
            if (!check_flag(state->flags, EXPECTING_OPERATOR))
                return -1;

            state->flags = 0 | EXPECTING_OPERAND | EXPECTING_OPEN_BRACE | EXPECTING_NEXT;

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
            if (!check_flag(state->flags, EXPECTING_CLOSE_BRACE))
                return -1;

            state->flags = 0 | EXPECTING_OPEN_BRACE 
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
                    if (state->set_ctr > state->sets_sz
                        || grouping_stack_ctr > STACK_SZ)
                            return -1;

                    /* Create GROUP(0) token, and reference in sets */
                    state->sets[state->set_ctr] = new_token(state, GROUPING, tokens[i]->start, 0);
                    
                    /* add it to output */
                    output[*output_ctr] = state->sets[grouping_stack_ctr];
                    
                    *output_ctr += 1;
                    state->set_ctr += 1;
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
            //  TODO: index_hints probably doesn't work
            // 
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
                    * TODO * DECREMENT ATOMICS CTR APPRIOTATELY
                    Should be able to parse Foo[::]
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
                    output[*output_ctr] = new_token(state, NULLTOKEN, 0, 0);
                    *output_ctr += 1;
                }

                output[*output_ctr] = new_token(state, INDEX_ACCESS, 0, 0);
                *output_ctr += 1;
            }
            else
            {
                /* 
                    place a grouping token after 
                    all the appriotate operations
                    have been placed. 
                */
                if (state->set_ctr > state->sets_sz
                    || grouping_stack_ctr > STACK_SZ)
                        return -1;
                /* 
                    setup token from allocated pool
                    and build a grouping operator
                */            
                /* using `start` as a position cordinate to determine the brace type */
                /* using `end` to signify how many values to pull from the stack */
                /* reference new token from pool into groups and output*/
                state->sets[state->set_ctr] = new_token(
                    state,
                    GROUPING,
                    head->start,
                    grouping_stack[grouping_stack_ctr] + 1
                );
                state->set_ctr += 1;
                
                /* place grouping operator on the output */
                output[*output_ctr] = state->sets[grouping_stack_ctr];
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
            if (check_flag(state->flags, EXPECTING_OPEN_BRACE))
                return -1;
            
            state->flags = 0 | EXPECTING_OPERAND 
                | EXPECTING_CLOSE_BRACE
                | EXPECTING_OPEN_BRACE
                | EXPECTING_NEXT;
            
            if (tokens[i]->type == BRACKET_OPEN)
                state->flags = state->flags | EXPECTING_COLON;

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
            if (!check_flag(state->flags, EXPECTING_COLON) 
                || index_colon_stack_ctr > STACK_SZ
                || index_colon_stack[index_colon_stack_ctr] > 2)
                return -1;

            state->flags = 0 | EXPECTING_OPERAND 
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
            search_token_buffer[0] = BRACKET_OPEN;

            if (get_last_inserted_token(
                operators,
                operators_ctr,
                search_token_buffer,
                1,
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
            if (!check_flag(state->flags, EXPECTING_COMMA)
                || grouping_stack_ctr > STACK_SZ)
                return -1;
            
            state->flags = 0 | EXPECTING_OPERAND 
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
        head = operators[operators_ctr - 1];
        /*
            any remaining params/brackets/braces are unclosed
            indiciate invalid expressions    
        */
        if (is_open_brace(head->type))
            return -1;
        
        if (*output_ctr > output_sz)
            return -1;
        
        output[*output_ctr] = head;
        
        *output_ctr += 1;
        operators_ctr -= 1;
    }

    return 0;
}

int8_t mk_atom_expr(
    struct Expr *ex,
    const char *line,
    const struct Token *from
){
    char *end;
    ex->free=0;

    if(from->type == WORD)
    {
        ex->type=SymExprT;
        ex->datatype=UndefT;

        memcpy(
            ex->inner.symbol,
            line + from->start,
            from->end - from->start
        );
    }

    else if(from->type == INTEGER)
    {
        ex->type=LiteralExprT;
        ex->datatype=IntT;
        errno = 0;

        ex->inner.value.literal.integer = strtol(line + from->start, &end, 10);
        
        if (errno != 0)
            return -1;
    }
    
    else if(from->type == STRING_LITERAL)
    {
        ex->type=LiteralExprT;
        ex->datatype=StringT;
    }

    else if(from->type == ADD)
    {

    }
    else
        return -1;

    return 0;
}

/* no special algorithms, just an equality test */
void determine_return_ty(struct Expr *bin) {
    if (bin->inner.bin.rhs->datatype == bin->inner.bin.lhs->datatype)
        bin->inner.bin.returns = bin->inner.bin.rhs->datatype;
    else
        bin->inner.bin.returns = UndefT;
}

#define EXPR_STACK_SZ 2048
int8_t postfix_into_tree(
    const char * line,
    const struct Token *tokens[],
    usize ntokens,
    const struct Token *fn_hints[],
    usize fn_hints_sz,
    struct ExprPool *pool
){
    char *end;
    struct Expr ex;
    struct Expr *phandle;
    struct Expr *stack[EXPR_STACK_SZ];
    uint16_t stack_ctr = 0;
    
    if (!line || !tokens || !pool || !fn_hints)
        return -1;

    usize fn_ctr=0;

    for (usize i=0; ntokens > i; i++)
    {
        memset(&ex, 0, sizeof(struct Expr));

        if (stack_ctr > EXPR_STACK_SZ)
            return -1;
        
        else if(tokens[i]->type == WORD)
        {
            ex.type=SymExprT;
            ex.datatype=UndefT;

            memcpy(
                ex.inner.symbol,
                line + tokens[i]->start,
                tokens[i]->end - tokens[i]->start
            );
            
            phandle = pool_push(pool, ex);
            if (phandle == 0)
               return -1;
            
            stack[stack_ctr] = phandle;
            stack_ctr += 1;
        }

        else if (tokens[i]->type == GROUPING) {
            if (tokens[i]->end > stack_ctr)
                return -1;
            
            ex.inner.value.literal.grouping.ptr = malloc(sizeof(struct Expr) * tokens[i]->end);
            ex.inner.value.literal.grouping.capacity = tokens[i]->end;
            ex.inner.value.literal.grouping.length = tokens[i]->end;
            ex.inner.value.literal.grouping.brace_type = brace_as_char(tokens[tokens[i]->start]->type);
            
            if (ex.inner.value.literal.grouping.ptr == 0)
                return -1;

            for (usize j=0; tokens[i]->end > j; j++) {
                ex.inner.value.literal.grouping.ptr[j] = stack[stack_ctr-1];
                stack_ctr -= 1;
            }
            
            phandle = pool_push(pool, ex);
            
            if (phandle == 0)
               return -1;

            stack[stack_ctr] = phandle;
        }
        else if(tokens[i]->type == INTEGER)
        {
            ex.type=LiteralExprT;
            ex.datatype=IntT;
            errno = 0;

            ex.inner.value.literal.integer = strtol(line + tokens[i]->start, &end, 10);
            
            if (errno != 0 || end != line + tokens[i]->end)
                return -1;
            
            phandle = pool_push(pool, ex);
            
            if (phandle == 0)
               return -1;

            stack[stack_ctr] = phandle;
            stack_ctr += 1;
        }
        
        else if(tokens[i]->type == STRING_LITERAL)
        {
            ex.type=LiteralExprT;
            ex.datatype=StringT;
            
            phandle = pool_push(pool, ex);
            
            if (phandle == 0)
               return -1;

            stack[stack_ctr] = phandle;
            stack_ctr += 1;
        }

        else if(is_bin_operator(tokens[i]->type))
        {
            if (2 >= stack_ctr)
                return -1;
            
            ex.type = BinaryExprT;
            ex.inner.bin.lhs = stack[stack_ctr-1];
            ex.inner.bin.rhs = stack[stack_ctr-2];
            ex.inner.bin.op = operation_from_token(tokens[i]->type);
            determine_return_ty(&ex);
            
            phandle = pool_push(pool, ex);
            
            if (phandle == 0)
               return -1;

            stack_ctr -= 2;
            stack[stack_ctr] = phandle;
            stack_ctr += 1;

        }
        else
            return -1;

    }
    return 0;

}

int8_t new_expr(char *line, struct Token tokens[], usize ntokens, struct Expr *expr) {
    postfix_expr(
        tokens,
        ntokens,
        struct Token **output,
        usize output_sz, 
        usize *output_ctr,
        struct ExprParserState *state,
        struct CompileTimeError *err
    )

    /*todo: create expr tree*/
    return 0;
}
