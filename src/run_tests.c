#include <stdio.h>
#include "libtest/CuTest.h"

CuSuite* CuGetSuite();
CuSuite* TestUtilTestSuite();
CuSuite* VecTestSuite();
CuSuite* LexerUnitTestSuite();
CuSuite* LexerHelpersUnitTestSuite();
CuSuite* PostFixUnitTestSuite();

void _onk_reset_buffer_hook(struct onk_test_buffers *ptr)
{
	onk_vec_clear(&ptr->src_tokens);
	onk_vec_clear(&ptr->postfix_token);
	ptr->msgbuf[0] = 0;
}

void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();
	struct onk_test_buffers buf;

	buf.msgbuf = malloc(8192);
	onk_vec_init(&buf.src_tokens, 256, sizeof(struct onk_token_t));
	onk_vec_init(&buf.postfix_token, 256, sizeof(struct onk_token_t));

	CuSuiteAddSuite(suite, CuGetSuite());
	CuSuiteAddSuite(suite, VecTestSuite());
	CuSuiteAddSuite(suite, TestUtilTestSuite());

	CuSuiteAddSuite(suite, LexerUnitTestSuite());
	CuSuiteAddSuite(suite, LexerHelpersUnitTestSuite());
	CuSuiteAddSuite(suite, PostFixUnitTestSuite());

	CuSuiteRun(suite, &buf);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);

	onk_vec_free(&buf.src_tokens);
	onk_vec_free(&buf.postfix_token);

	free(buf.msgbuf);
	printf("%s\n", output->buffer);
}

int main(void)
{ RunAllTests(); }
