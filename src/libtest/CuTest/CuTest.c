#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libtest/CuTest.h"

/*-------------------------------------------------------------------------*
 * CuTest
 *-------------------------------------------------------------------------*/

void CuTestInit(
	CuTest* t,
	const char* name,
	enum CuTestType type,
	union TestFn function)
{
	t->name = CuStrCopy(name);
	t->failed = 0;
	t->ran = 0;
	t->message = NULL;
	t->fntype = type;
	t->func = function;
	t->jumpBuf = NULL;
}

int8_t NewTestFn(union TestFn *dest, enum CuTestType type, void * fn)
{
	switch(type)
	{
		case CuTestType:
			dest->norm = fn;
			break;
		case ClonkTestType:
			dest->buffered = fn;
			break;

		default: return -1;
	}
	return 0;
}
CuTest* CuTestNew(
	const char* name,
	enum CuTestType type,
	void * fn)
{
	union TestFn func;
	NewTestFn(&func, type, fn);

	CuTest* tc = CU_ALLOC(CuTest);
	CuTestInit(tc, name, type, func);
	return tc;
}

void CuTestDelete(CuTest *t)
{
        if (!t) return;
        free(t->name);
        free(t);
}

void _reset(struct onk_test_state_t * state) {
	state->parser_i = 0;
	onk_parser_reset(&state->parser);

	onk_vec_clear(&state->src_tokens);
	onk_vec_clear(&state->postfix_token);
	onk_vec_clear(&state->parser_expect);
	onk_vec_clear(&state->lexer_expect);
}

void CuTestRun(CuTest* tc, struct onk_test_state_t *ptr)
{
	jmp_buf buf;
	tc->jumpBuf = &buf;

	if (setjmp(buf) == 0)
	{
		tc->ran = 1;
		switch (tc->fntype)
		{
			case ClonkTestType:
				_reset(ptr);
				(tc->func.buffered)(tc, ptr);
				break;
			case CuTestType:
				(tc->func.norm)(tc);
				break;
		    default:
				CuFail(tc, "Misconfigured Test");
				break;
		}
	}
	printf("RAN: %s\n", tc->name);
	tc->jumpBuf = 0;
}

static void CuFailInternal(CuTest* tc, const char* file, int line, CuString* string)
{
	char buf[HUGE_STRING_LEN];

	sprintf(buf, "%s:%d: ", file, line);
	CuStringInsert(string, buf, 0);

	tc->failed = 1;
	tc->message = string->buffer;
	if (tc->jumpBuf != 0) longjmp(*(tc->jumpBuf), 0);
}

void CuFail_Line(CuTest* tc, const char* file, int line, const char* message2, const char* message)
{
	CuString string;

	CuStringInit(&string);
	if (message2 != NULL) 
	{
		CuStringAppend(&string, message2);
		CuStringAppend(&string, ": ");
	}
	CuStringAppend(&string, message);
	CuFailInternal(tc, file, line, &string);
}

void CuAssert_Line(CuTest* tc, const char* file, int line, const char* message, int condition)
{
	if (condition) return;
	CuFail_Line(tc, file, line, NULL, message);
}

void CuAssertStrEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, 
	const char* expected, const char* actual)
{
	CuString string;
	if ((expected == NULL && actual == NULL) ||
	    (expected != NULL && actual != NULL &&
	     strcmp(expected, actual) == 0))
	{
		return;
	}

	CuStringInit(&string);
	if (message != NULL) 
	{
		CuStringAppend(&string, message);
		CuStringAppend(&string, ": ");
	}
	CuStringAppend(&string, "expected <");
	CuStringAppend(&string, expected);
	CuStringAppend(&string, "> but was <");
	CuStringAppend(&string, actual);
	CuStringAppend(&string, ">");
	CuFailInternal(tc, file, line, &string);
}

void CuAssertIntEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, 
	int expected, int actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected <%d> but was <%d>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}

void CuAssertDblEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, 
	double expected, double actual, double delta)
{
	char buf[STRING_MAX];
 	if (fabs(expected - actual) <= delta) return;
	sprintf(buf, "expected <%f> but was <%f>", expected, actual); 

	CuFail_Line(tc, file, line, message, buf);
}

void CuAssertPtrEquals_LineMsg(CuTest* tc, const char* file, int line, const char* message, 
	void* expected, void* actual)
{
	char buf[STRING_MAX];
	if (expected == actual) return;
	sprintf(buf, "expected pointer <0x%p> but was <0x%p>", expected, actual);
	CuFail_Line(tc, file, line, message, buf);
}


