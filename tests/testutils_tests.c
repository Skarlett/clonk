#include "testutils.h"

void mk_toks(struct onk_token_t token[], const enum onk_lexicon_t ty[]) {
  for (usize i = 0 ;; i++)
    if (ty[i] == 0)
      break;
    else
      token[i].type = ty[i];
}

void __test__check_tokens(CuTest* tc) {
     struct onk_token_t toks[8];

     const static enum onk_lexicon_t check_list[][16] = {
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_DIV_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_ADD_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_MUL_TOKEN, ONK_SUB_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_ADD_TOKEN, 0},
    };


    for (uint8_t i=0; 6 > i; i++){
        mk_toks(toks, check_list[i]);
        onk_assert_tokens(tc, "n/a", "", toks, check_list[i]);
    }
}


CuSuite* TestUtilTestSuite(void) {
	CuSuite* suite = CuSuiteNew();
    return suite;
}