#include "testutils.h"

void mk_toks(struct Token token[], const enum Lexicon ty[]) {
  for (usize i = 0 ;; i++)
    if (ty[i] == 0)
      break;
    else
      token[i].type = ty[i];
}

void __test__check_tokens(CuTest* tc) {
     struct Token toks[8];

     const static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, INTEGER, MUL, ADD, 0},
        {INTEGER, INTEGER, DIV, INTEGER, ADD, 0},
        {WORD, WORD, ADD, WORD, WORD, MUL, SUB, 0},
        {INTEGER, INTEGER, MUL, INTEGER, ADD, 0},
        {INTEGER, INTEGER, INTEGER, MUL, ADD, 0},
    };


    for (uint8_t i=0; 6 > i; i++){
        mk_toks(toks, check_list[i]);
        AssertTokens(tc, "n/a", "", toks, check_list[i]);
    }
}


CuSuite* TestUtilTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__check_tokens);
    return suite;
}