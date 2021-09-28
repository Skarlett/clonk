#include "../CuTest.h"
#include "../common.h"
#include "../../src/prelude.h"
#include "../../src/parser/expr/expr.h"

void __test__fnmasks_no_function(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "1 + 2 + 3";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);
    CuAssertTrue(tc, input_sz == 5);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 0);
    CuAssertTrue(tc, output_ctr == input_sz);

    for (usize i=0; input_sz > i; i++)
        // assert memory addresses are the same
        CuAssertTrue(tc, &input[i] == output[i]);
}

void __test__fnmasks_empty_function(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "foo()";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);
    CuAssertTrue(tc, input_sz == 3);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 2);
}

void __test__fnmasks_with_args(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "foo(a, b, c)";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);
    CuAssertTrue(tc, input_sz == 8);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 7);
}

void __test__fnmasks_with_operators_in_args(CuTest* tc) {
    struct Token input[16],
        masks[4],
        *output[16];
    

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "foo(a == 2, b+2, c)";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);
    CuAssertTrue(tc, input_sz == 12);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 11);
}


void __test__fnmasks_with_parthesis_in_args(CuTest* tc) {
      struct Token input[64],
        masks[4],
        *output[16];
    

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;
    
    static char *source = "foo(((a == 2)), (b+2)*5, c)";
    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);

    CuAssertTrue(tc, input_sz == 20);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            4,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
        ) == 0
    );

    CuAssertTrue(tc, masks_ctr == 1);
    CuAssertTrue(tc, output_ctr == 1);
    CuAssertTrue(tc, output[0]->type == FNMASK);
    CuAssertTrue(tc, output[0]->start == 0);
    CuAssertTrue(tc, output[0]->end == 19);
}

void __test__fnmasks_multi_empty(CuTest* tc) {
    struct Token input[64],
        masks[4],
        *output[16];
    

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo() + foo()";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);
    CuAssertTrue(tc, input_sz == 7);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
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

void __test__fnmasks_multi_with_args(CuTest* tc) {
      struct Token input[64],
        masks[4],
        *output[16];
    
    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo(a, b, c) + foo(a, b, c)";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);
    CuAssertTrue(tc, input_sz == 17);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
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

void __test__fnmasks_multi_with_operators_in_args(CuTest* tc) {
      struct Token input[64],
        masks[4],
        *output[16];
    

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo(a + 2, b-2, c) + foo(a^2, b%2, c*0)";
    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);

    CuAssertTrue(tc, input_sz == 27);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
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

void __test__fnmasks_multi_with_parathesis_in_args(CuTest* tc) {
    struct Token input[64],
        masks[4],
        *output[16];
    

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo(((((a^2) + (b%2), c*0)))) + 1";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);
    CuAssertTrue(tc, input_sz == 26);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
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

void __test__fnmasks_with_unbalanced_parthesis_left_of_args(CuTest* tc) {
      struct Token input[46],
        masks[4],
        *output[16];

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    
    static char *source = "foo(()";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);
    CuAssertTrue(tc, input_sz == 4);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            4,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
        ) == -1
    );
}

void __test__fnmasks_with_unbalanced_parthesis_right_of_args(CuTest* tc) {
    struct Token input[32],
        masks[4],
        *output[16];

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo())";
    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);

    CuAssertTrue(tc, input_sz == 19);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            4,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
        ) == -1
    );
}

void __test__fnmasks_with_application(CuTest* tc) {
      struct Token input[46],
        masks[4],
        *output[16];
    

    usize input_sz = 0;
    usize masks_ctr = 0;
    usize output_ctr = 0;

    static char *source = "foo().length + 1\0";

    CuAssertTrue(tc, tokenize(source, input, &input_sz, NULL) == 0);

    CuAssertTrue(tc, input_sz == 26);

    CuAssertTrue(tc,
        mk_fnmask_tokens(
            output,
            16,
            &output_ctr,
            input,
            input_sz,
            masks,
            4,
            &masks_ctr,
            NULL
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


CuSuite* FnMaskUnitTestSuite(void) {
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

    //SUITE_ADD_TEST(suite, __test__fnmasks_with_application);

    return suite;
}
