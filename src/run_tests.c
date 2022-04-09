#include <stdio.h>
#include "libtest/CuTest.h"

CuSuite* CuGetSuite();
CuSuite* TestUtilTestSuite();
CuSuite* VecTestSuite();
CuSuite* LexerUnitTestSuite();
CuSuite* LexerHelpersUnitTestSuite();
CuSuite* PostFixUnitTestSuite();


#define GLOB_SIZE 2048
void _onk_init_glob_buffer(struct _onk_test_buffers *buf)
{
	buf->msgbuf = malloc(sizeof(char) * GLOB_SIZE);
	buf->msg_sz = GLOB_SIZE;
	buf->msg_cursor = 0;

	onk_vec_init(&buf->src_tokens, 256, sizeof(struct onk_token_t));
	onk_vec_init(&buf->postfix_token, 256, sizeof(struct onk_token_t));
	onk_vec_init(&buf->postfix_token, 256, sizeof(struct onk_token_t));
}

void _onk_reset_glob_buffer(struct _onk_test_buffers *buf)
{
	buf->msg_cursor = 0;
	buf->msgbuf[0] = 0;

	onk_vec_clear(&buf->src_tokens);
	onk_vec_clear(&buf->postfix_token);
	onk_vec_clear(&buf->src_tokens);
}


void RunAllTests(void)
{
	CuString *output = CuStringNew();
	CuSuite* suite = CuSuiteNew();

	CuSuiteAddSuite(suite, CuGetSuite());
	CuSuiteAddSuite(suite, TestUtilTestSuite());
	CuSuiteAddSuite(suite, VecTestSuite());
	CuSuiteAddSuite(suite, LexerUnitTestSuite());
	CuSuiteAddSuite(suite, LexerHelpersUnitTestSuite());
	CuSuiteAddSuite(suite, PostFixUnitTestSuite());

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
}

int main(void)
{
	_onk_init_glob_buffer(&BUFFERS);
	RunAllTests();
}
