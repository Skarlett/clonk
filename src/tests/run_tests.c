#include "clonk.h"
#include "libtest/CuTest.h"

/********************************/
/*  Test the function harness   */
/********************************/
CuSuite* CuGetSuite();
CuSuite* CuGetCtxSuite();

/******************/
/*  Test libtest  */
/******************/
CuSuite* OnkMaskAssertSuite();
CuSuite* OnkAssertTests();

/****************/
/*  Test utils  */
/****************/
CuSuite* OnkVecTests();
CuSuite* OnkQueueTest();
CuSuite* OnkMergeSortTests();

/*****************/
/*   Test lexer  */
/*****************/
CuSuite* LexerUnitTests();

CuSuite* LexerHarnessUBTests();
CuSuite* LexerHarnessLogicTests();

/*****************/
/*  Test parser  */
/*****************/
CuSuite* PostfixTests();

/* int8_t onk_parser_init( */
/*   struct onk_parser_state_t *state, */
/*   uint16_t *i */
/* ); */

/* int8_t onk_parser_free(struct onk_parser_state_t *state); */
/* int8_t onk_parser_reset(struct onk_parser_state_t *state); */


/* FORWARD DECLARED in CuTest.c*/
/* Ran on every test finishing */
void _onk_reset_buffer_hook(CuTest *tc, struct onk_test_state_t *ptr)
{
	ptr->parser_i = 0;
	onk_parser_init(&ptr->parser, &ptr->parser_i);

	onk_vec_clear(&ptr->src_tokens);
	onk_vec_clear(&ptr->postfix_token);
	onk_vec_clear(&ptr->src_tokens);
	onk_vec_clear(&ptr->postfix_token);
}

/* FORWARD DECLARED in CuTest.c*/
/* Ran on every test finishing */
void _onk_init_test_buffer_hook(struct onk_test_state_t *buf)
{
	onk_vec_init(&buf->src_tokens, 256, sizeof(struct onk_token_t));
	onk_vec_init(&buf->postfix_token, 256, sizeof(struct onk_token_t));
}


void RunAllTests(void)
{
	struct onk_test_state_t buf;

	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	_onk_init_test_buffer_hook(&buf);

	CuSuiteAddSuite(suite, CuGetSuite());

	CuSuiteAddSuite(suite, OnkMaskAssertSuite());
	CuSuiteAddSuite(suite, OnkAssertTests());

	CuSuiteAddSuite(suite, OnkVecTests());
	CuSuiteAddSuite(suite, OnkQueueTest());
	CuSuiteAddSuite(suite, OnkMergeSortTests());

	CuSuiteAddSuite(suite, LexerUnitTests());

	CuSuiteAddSuite(suite, LexerHarnessUBTests());
	CuSuiteAddSuite(suite, LexerHarnessLogicTests());

	CuSuiteAddSuite(suite, PostfixTests());

	CuSuiteRun(suite, &buf);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);

	printf("%s\n", output->buffer);

	_onk_free_test_buffer_hook(&buf);
}

int main(void)
{ RunAllTests(); }
