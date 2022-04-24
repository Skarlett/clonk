#include <stdio.h>
#include "libtest/CuTest.h"

/********************************/
/*  Test the function harness   */
/********************************/
CuSuite* CuGetSuite();
CuSuite* ClonkTestFnSuite();

/******************/
/*  Test libtest  */
/******************/
CuSuite* OnkAssertSuite();
CuSuite* OnkTokenMaskAssertSuite();

/****************/
/*  Test utils  */
/****************/
CuSuite* VecTestSuite();

/*****************/
/*  Test parser  */
/*****************/
CuSuite* CuLexerHarnessInput();
CuSuite* LexerUnitTestSuite();
CuSuite* LexerHelpersUnitTestSuite();
CuSuite* PostFixUnitTestSuite();



/* FORWARD DECLARED in CuTest.c*/
/* Ran on every test finishing */
void _onk_reset_buffer_hook(struct onk_test_buffers *ptr)
{
	onk_vec_clear(&ptr->src_tokens);
	onk_vec_clear(&ptr->postfix_token);
	ptr->msgbuf[0] = '\0';
}

/* FORWARD DECLARED in CuTest.c*/
/* Ran on every test finishing */
void _onk_init_test_buffer_hook(struct onk_test_buffers *buf)
{
	buf->msgbuf = malloc(8192);
	buf->msg_capacity = 8192;
	onk_vec_init(&buf->src_tokens, 256, sizeof(struct onk_token_t));
	onk_vec_init(&buf->postfix_token, 256, sizeof(struct onk_token_t));
}

/* FORWARD DECLARED in CuTest.c*/
/* Ran on every test finishing */
void _onk_free_test_buffer_hook(struct onk_test_buffers *buf)
{
	onk_vec_free(&buf->src_tokens);
	onk_vec_free(&buf->postfix_token);
	free(buf->msgbuf);
}


void TestHarness(CuSuite* suite)
{
	CuSuiteAddSuite(suite, CuGetSuite());
	CuSuiteAddSuite(suite, OnkAssertSuite());
	CuSuiteAddSuite(suite, OnkTokenMaskAssertSuite());
}

void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();
	struct onk_test_buffers buf;

	_onk_init_test_buffer_hook(&buf);

	TestHarness(suite);

	CuSuiteAddSuite(suite, VecTestSuite());
	CuSuiteAddSuite(suite, LexerUnitTestSuite());
	CuSuiteAddSuite(suite, LexerHelpersUnitTestSuite());
	CuSuiteAddSuite(suite, PostFixUnitTestSuite());

	CuSuiteRun(suite, &buf);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);

	printf("%s\n", output->buffer);

	_onk_free_test_buffer_hook(&buf);
}

int main(void)
{ RunAllTests(); }
