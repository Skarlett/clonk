#include <string.h>
#include <stdio.h>
#include "../src/parser/lexer/lexer.h"
#include "../src/parser/lexer/helpers.h"
#include "../src/parser/expr/expr.h"
#include "../src/parser/expr/debug.h"
#include "../src/prelude.h"
#include "../src/parser/error.h"

#include "CuTest.h"



// /*
//    Prove incremential changes cause comparsion failure.
// */
// void __test__sanity_expr_cmp(CuTest* tc)
// {
//     struct Expr a, b;
    
//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     a.inner.uni.interal_data.symbol.tag = ValueTag;
//     a.inner.uni.interal_data.symbol.inner.value.type = IntT;
//     a.inner.uni.interal_data.symbol.inner.value.data.integer = 1;

//     CuAssertTrue(tc, cmpexpr(&a, &a) == 1);

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;
//     b.inner.uni.interal_data.symbol.tag = VariableTag;
//     b.inner.uni.interal_data.symbol.inner.variable = "a";
    
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     CuAssertTrue(tc, cmpexpr(&b, &b) == 1);

//     a.inner.uni.interal_data.symbol.tag = VariableTag;
//     a.inner.uni.interal_data.symbol.inner.variable = "b";

//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;

//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     a.inner.uni.interal_data.symbol.inner.variable = "a";
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
//     a.type = UndefinedExprT;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     b.type = UndefinedExprT;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
//     a.type = UniExprT;
//     b.type = UniExprT;


//     a.inner.uni.op = UniOpNop;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
    
//     a.inner.uni.op = UniValue;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

//     a.inner.uni.interal_data.symbol.tag = NullTag;
//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);

//     b.inner.uni.interal_data.symbol.tag = NullTag;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;

//     a.inner.uni.interal_data.symbol.tag = ValueTag;
//     b.inner.uni.interal_data.symbol.tag = ValueTag;

//     a.inner.uni.interal_data.symbol.inner.value.type = IntT;
//     b.inner.uni.interal_data.symbol.inner.value.type = IntT;
//     a.inner.uni.interal_data.symbol.inner.value.data.integer = 1;
//     b.inner.uni.interal_data.symbol.inner.value.data.integer = 2;

//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     b.inner.uni.interal_data.symbol.inner.value.data.integer = 1;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;

//     b.inner.uni.interal_data.symbol.inner.value.type = StringT;
//     b.inner.uni.interal_data.symbol.inner.value.data.string.capacity = 0;
//     b.inner.uni.interal_data.symbol.inner.value.data.string.length = 5;
//     b.inner.uni.interal_data.symbol.inner.value.data.string.ptr = "data\0";
    
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     a.inner.uni.interal_data.symbol.inner.value.type = StringT;
//     a.inner.uni.interal_data.symbol.inner.value.data.string.capacity = 0;
//     a.inner.uni.interal_data.symbol.inner.value.data.string.length = 5;
//     a.inner.uni.interal_data.symbol.inner.value.data.string.ptr = "data\0";
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
    
//     struct Expr bin;
//     bin.type = BinExprT;
//     bin.inner.bin.lhs = &a;
//     bin.inner.bin.rhs = &b;

//     CuAssertTrue(tc, cmpexpr(&bin, &bin) == 1);
//     CuAssertTrue(tc, cmpexpr(&a, &bin) == 0);

//     //TODO add test cases for fncalls
// }



int __test__fnmasks_no_function(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "1 + 2 + 3";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);
    CuAssertTrue(tc, input_sz == 5);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 0);
    CuAssertTrue(tc, output_ctr == input_sz);

    for (usize i=0; input_sz > i; i++)
        // assert memory addresses are the same
        CuAssertTrue(tc, &input[i] == output[i]);
}

int __test__fnmasks_empty_function(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "foo()";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);
    CuAssertTrue(tc, input_sz == 3);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 2);
}

int __test__fnmasks_with_args(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "foo(a, b, c)";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);
    CuAssertTrue(tc, input_sz == 8);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 7);
}

int __test__fnmasks_with_operators_in_args(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "foo(a == 2, b+2, c)";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);
    CuAssertTrue(tc, input_sz == 12);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 11);
}


int __test__fnmasks_with_parthesis_in_args(CuTest* tc) {
      struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "foo(((a == 2)), (b+2)*5, c)";
    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);

    CuAssertTrue(tc, input_sz == 20);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            4,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 19);
}

int __test__fnmasks_multi_empty(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo() + foo()";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);
    CuAssertTrue(tc, input_sz == 7);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 2);
    CuAssertTrue(tc, output_ctr == 3);
    
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 2);
    
    CuAssertTrue(tc, output[1]->type == ADD);
    CuAssertTrue(tc, output[1]->start == 6);
    CuAssertTrue(tc, output[1]->end == 6);

    CuAssertTrue(tc, output[2]->type == FNMASK);
    CuAssertTrue(tc, output[2]->start == 4);
    CuAssertTrue(tc, output[2]->end == 6);

    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, input[output[0]->start].type == WORD);
    CuAssertTrue(tc, input[output[0]->start].start == 0);
    CuAssertTrue(tc, input[output[0]->start].end == 2);

    CuAssertTrue(tc, input[0].type == WORD);
    CuAssertTrue(tc, input[0].start == 0);
    CuAssertTrue(tc, input[0].end == 2);

    CuAssertTrue(tc, input[1].type == PARAM_OPEN);
    CuAssertTrue(tc, input[1].start == 3);
    CuAssertTrue(tc, input[1].end == 3);

    CuAssertTrue(tc, input[2].type == PARAM_CLOSE);
    CuAssertTrue(tc, input[2].start == 4);
    CuAssertTrue(tc, input[2].end == 4);

    CuAssertTrue(tc, output[0]->end == 2);
    CuAssertTrue(tc, input[output[0]->end].type == PARAM_CLOSE);
    CuAssertTrue(tc, input[output[0]->end].start == 4);
    CuAssertTrue(tc, input[output[0]->end].end == 4);

    CuAssertTrue(tc, input[3].type == ADD);
    CuAssertTrue(tc, input[3].start == 6);
    CuAssertTrue(tc, input[3].end == 6);

    CuAssertTrue(tc, output[2]->start == 4);
    CuAssertTrue(tc, input[output[2]->start].type == WORD);
    CuAssertTrue(tc, input[output[2]->start].start == 8);
    CuAssertTrue(tc, input[output[2]->start].end == 10);

    CuAssertTrue(tc, input[4].type == WORD);
    CuAssertTrue(tc, input[4].start == 8);
    CuAssertTrue(tc, input[4].end == 10);

    CuAssertTrue(tc, input[5].type == PARAM_OPEN);
    CuAssertTrue(tc, input[5].start == 11);
    CuAssertTrue(tc, input[5].end == 11);

    CuAssertTrue(tc, input[6].type == PARAM_CLOSE);
    CuAssertTrue(tc, input[6].start == 12);
    CuAssertTrue(tc, input[6].end == 12);
    
    CuAssertTrue(tc, output[2]->end == 6);
    CuAssertTrue(tc, input[output[2]->end].type == PARAM_CLOSE);
    CuAssertTrue(tc, input[output[2]->end].start == 12);
    CuAssertTrue(tc, input[output[2]->end].end == 12);
}

int __test__fnmasks_multi_with_args(CuTest* tc) {
      struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo(a, b, c) + foo(a, b, c)";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);
    CuAssertTrue(tc, input_sz == 17);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 2);
    CuAssertTrue(tc, output_ctr == 3);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 8);
    
    CuAssertTrue(tc, output[1]->type == ADD);
    CuAssertTrue(tc, output[1]->start == 13);
    CuAssertTrue(tc, output[1]->end == 13);

    CuAssertTrue(tc, output[2]->type == FNMASK);
    CuAssertTrue(tc, output[2]->start == 9);
    CuAssertTrue(tc, output[2]->end == 17);
}

int __test__fnmasks_multi_with_operators_in_args(CuTest* tc) {
      struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo(a + 2, b-2, c) + foo(a^2, b%2, c*0)";
    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);

    CuAssertTrue(tc, input_sz == 27);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 2);
    CuAssertTrue(tc, output_ctr == 3);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 11);
    
    CuAssertTrue(tc, output[1]->type == ADD);
    CuAssertTrue(tc, output[1]->start == 19);
    CuAssertTrue(tc, output[1]->end == 19);

    CuAssertTrue(tc, output[2]->type == FNMASK);
    CuAssertTrue(tc, output[2]->start == 13);
    CuAssertTrue(tc, output[2]->end == 26);
}

int __test__fnmasks_multi_with_parathesis_in_args(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo(((((a^2) + (b%2), c*0)))) + 1";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);
    CuAssertTrue(tc, input_sz == 26);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 3);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 23);
    
    CuAssertTrue(tc, output[1]->type == ADD);
    CuAssertTrue(tc, output[1]->start == 30);
    CuAssertTrue(tc, output[1]->end == 30);

    CuAssertTrue(tc, output[2]->type == INTEGER);
    CuAssertTrue(tc, output[2]->start == 32);
    CuAssertTrue(tc, output[2]->end == 32);
}

int __test__fnmasks_with_unbalanced_parthesis_left_of_args(CuTest* tc) {
      struct Token input[46],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    
    static char *source = "foo(((a == 2), (b+2)*5, c)";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);
    CuAssertTrue(tc, input_sz == 19);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            4,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == -1
    );
}

int __test__fnmasks_with_unbalanced_parthesis_right_of_args(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo((a == 2), (b+2)*5, c))";
    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);

    CuAssertTrue(tc, input_sz == 19);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            4,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == -1
    );
}

int __test__fnmasks_with_application(CuTest* tc) {
      struct Token input[46],
        masks[4],
        *output[16];
    
    struct CompileTimeError err;

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo().length + 1\0";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, &err) == 0);

    CuAssertTrue(tc, input_sz == 26);

    CuAssertTrue(tc,
        create_fnmasks(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 3);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 2);
    
    CuAssertTrue(tc, output[1]->type == DOT);
    CuAssertTrue(tc, output[1]->start == 5);
    CuAssertTrue(tc, output[1]->end == 5);

    CuAssertTrue(tc, output[2]->type == WORD);
    CuAssertTrue(tc, output[2]->start == 6);
    CuAssertTrue(tc, output[2]->end == 11);

    CuAssertTrue(tc, output[3]->type == ADD);
    CuAssertTrue(tc, output[3]->start == 13);
    CuAssertTrue(tc, output[3]->end == 13);

    CuAssertTrue(tc, output[4]->type == INTEGER);
    CuAssertTrue(tc, output[4]->start == 15);
    CuAssertTrue(tc, output[4]->end == 15);
}


void __test__order_precedence(CuTest* tc) {
    usize ntokens=0, nqueue=0;

    
    struct Token tokens[32],
        *queue[32],
        *masks[2];
    
    static char * line[] = {
        "1 + 2",
        "1 + 3 * 4",
        "1 / 2 + 2",
        "(a - 2) * 3",
        "a + b - c * d",
        "1 * 2 + 3",
        0
    };
    
    static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
        {INTEGER, INTEGER, INTEGER, DIV, ADD},
        {WORD, INTEGER, SUB, INTEGER, INTEGER, MUL},
        {WORD, WORD, WORD, WORD, ADD, SUB, MUL},
        {INTEGER, INTEGER, INTEGER, MUL, ADD},
    };

    for (usize i=0; 6 > i; i++) {
        CuAssertTrue(tc, tokenize(line[i], tokens, &ntokens, NULL) == 0);
        nqueue = construct_postfix_queue(tokens, ntokens, queue, 32, masks, 2);
        CuAssertTrue(tc, __check_tokens_by_ref(queue, check_list[i], (usize)nqueue));
    }

}

void __test__order_precedence_left_assoc(CuTest* tc) {
    usize ntokens=0, nqueue=0;
    
    static enum Lexicon answer[][5] = {
        {INTEGER, INTEGER, INTEGER, SUB, SUB},
        {INTEGER, INTEGER, INTEGER, DIV, DIV}
    };
    
    static char * words[][12] = {
        {"3", "2", "1", "-", "-", 0},
        {"12", "2", "2", "/", "/", 0},
    };

    struct Token tokens[32],
        *queue[32],
        *masks[2];

    static char * line[] = {
        "3 - 2 - 1", /* 0 */
        "12 / 2 / 3", /* 2 */
        0
    };
    for (usize i=0; 2 > i; i++) {
        CuAssertTrue(tc, tokenize(line[i], tokens, &ntokens, NULL) == 0);
        nqueue = construct_postfix_queue(tokens, ntokens, queue, 32, masks, 2);
        CuAssertTrue(tc, nqueue == 5);
        CuAssertTrue(tc, __check_tokens_by_ref(queue, answer[i]));

        for (usize j=0; nqueue > j; j++) {
            CuAssertTrue(tc, strncmp(
                line+queue[j]->start,
                words[j],
                queue[j]->end-queue[j]->start
            ) == 0);
        }
    }
}


void __test__order_precedence_right_assoc(CuTest* tc) {
    static enum Lexicon answer[] = {INTEGER, INTEGER, INTEGER, POW, POW};
    static char * words[] = {"3", "2", "1", "^", "^", 0};

    usize ntokens=0,
        nqueue=0;

    struct Token tokens[32],
        *queue[32],
        *masks[2];

    static char * line = "1 ^ 2 ^ 3"; /* 8 */

    CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 5);

    nqueue = construct_postfix_queue(tokens, ntokens, queue, 32, masks, 2);
    CuAssertTrue(tc, __check_tokens_by_ref(queue, answer));
}

void __test__order_precedence_not_op(CuTest* tc) {
    usize ntokens=0,
        nqueue=0;
    static enum Lexicon answers[] = {NOT, WORD, WORD, ADD, NOT, WORD, SUB};

    struct Token tokens[32],
        *queue[32],
        *masks[2];

    static char * line = "!a + b - !c";
    CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 7);

    nqueue = construct_postfix_queue(tokens, ntokens, queue, 32, masks, 2);
    
    for (usize i=0; nqueue > i; i++) {
        CuAssertTrue(tc, queue[i]->type == answers[i]);
    }
}

void __test__order_precedense_apply_op(CuTest* tc) {
    usize ntokens, nqueue;
    static enum Lexicon answer[] = {WORD, WORD, WORD, DOT, DOT};
    static char * words[] = {"last", "middle", "first", ".", ".", 0};    
    struct Token tokens[32],
        *queue[32],
        *masks[2];
    
    static char * line = "first.middle.last";
    CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
    CuAssertTrue(tc, ntokens == 5);

    nqueue = construct_postfix_queue(tokens, ntokens, queue, 32, masks, 2);
    CuAssertTrue(tc, nqueue == 5);

    for (usize i=0; nqueue > i; i++) {
        CuAssertTrue(tc, queue[i]->type == answer[i]);
        CuAssertTrue(tc, strncmp(
            line+queue[i]->start,
            words[i],
            queue[i]->end-queue[i]->start
        ) == 0);
    }
}

void __test__order_precedense_with_fncall(CuTest* tc) {
    usize ntokens, nqueue;
    
    struct Token tokens[32],
        *queue[32],
        *masks[2];
    
    static char * line = "first().middle.last()";
    CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);

    CuAssertTrue(tc, ntokens == 9);

    construct_postfix_queue(tokens, ntokens, queue, 32, masks, 2);
}

// int __test__fnmasks_with_application(CuTest* tc) {
//       struct Token input[46],
//         masks[4],
//         *output[16];
    
//     struct CompileTimeError err;

//     usize input_sz = 0;
//     usize masks_ctr = 0;
//     usize output_ctr = 0;

//     static char *source = "foo().length + 1";
    
//     CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);

//     CuAssertTrue(tc, input_sz == 26);

//     CuAssertTrue(tc,
//         create_fnmasks(
//             output,
//             16,
//             &output_ctr,
//             input,
//             input_sz,
//             masks,
//             4,
//             &masks_ctr
//         ) == 0
//     );

//     CuAssertTrue(tc, masks_ctr == 1);
 
// }

CuSuite* ExprUnitTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__fnmasks_no_function);
    SUITE_ADD_TEST(suite, __test__fnmasks_empty_function);
    SUITE_ADD_TEST(suite, __test__fnmasks_with_args);
    SUITE_ADD_TEST(suite, __test__fnmasks_with_operators_in_args);
    SUITE_ADD_TEST(suite, __test__fnmasks_with_parthesis_in_args);
    
    SUITE_ADD_TEST(suite, __test__fnmasks_multi_empty);
    SUITE_ADD_TEST(suite, __test__fnmasks_multi_with_operators_in_args);
    SUITE_ADD_TEST(suite, __test__fnmasks_multi_with_parathesis_in_args);

    SUITE_ADD_TEST(suite, __test__fnmasks_with_unbalanced_parthesis_left_of_args);
    SUITE_ADD_TEST(suite, __test__fnmasks_with_unbalanced_parthesis_right_of_args);

    SUITE_ADD_TEST(suite, __test__fnmasks_with_application);
    SUITE_ADD_TEST(suite, __test__order_precedence_not_op);
    //SUITE_ADD_TEST(suite, __test__order_precedence);

    return suite;
}
