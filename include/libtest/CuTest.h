#ifndef CU_TEST_H
#define CU_TEST_H

#include <setjmp.h>
#include <stdarg.h>
#include "clonk.h"
#include "lexer.h"
#include "parser.h"

#define CUTEST_VERSION  "CuTest 1.5 - clonk derivation"

/* CuString */

char* CuStrAlloc(int size);
char* CuStrCopy(const char* old);

#define CU_ALLOC(TYPE)		((TYPE*) malloc(sizeof(TYPE)))

#define HUGE_STRING_LEN	8192
#define STRING_MAX		256
#define STRING_INC		256

typedef struct
{
	int length;
	int size;
	char* buffer;
} CuString;

void CuStringInit(CuString* str);
CuString* CuStringNew(void);
void CuStringRead(CuString* str, const char* path);
void CuStringAppend(CuString* str, const char* text);
void CuStringAppendChar(CuString* str, char ch);
void CuStringAppendFormat(CuString* str, const char* format, ...);
void CuStringInsert(CuString* str, const char* text, int pos);
void CuStringResize(CuString* str, int newSize);
void CuStringDelete(CuString* str);

/* CuTest */
typedef struct CuTest CuTest;
typedef void (*CuTestFn)(CuTest *tc);

struct onk_test_state_t {
	/* Used for mock tests */
	struct onk_parser_state_t parser;
	uint16_t parser_i;


	/* Initialized/heap Vec<struct Token> */
	struct onk_vec_t src_tokens;

	/* Initialized/heap Vec<struct Token> */
	struct onk_vec_t postfix_token;

	/* Initialized/heap Vec<struct onk_token_desc_t> */
	struct onk_vec_t parser_expect;

	/* Initialized/heap Vec<struct onk_token_desc_t> */
	struct onk_vec_t lexer_expect;
};

typedef void (*ClonkTestFn)(CuTest *tc, struct onk_test_state_t *ptr);

union TestFn {
	CuTestFn norm;
	ClonkTestFn buffered;
};

enum CuTestType {
  ClonkTestType,
  CuTestType,
};

struct CuTest
{
	enum CuTestType fntype;
	union TestFn func;

	char* name;
	int failed;
	int ran;
	const char* message;
	jmp_buf *jumpBuf;
};

void CuTestInit(
	CuTest* t,
	const char* name,
	enum CuTestType type,
	union TestFn function
);

int8_t NewTestFn(union TestFn *dest, enum CuTestType type, void * fn);

CuTest* CuTestNew(const char* name, enum CuTestType type,
	void * fn);

void CuTestRun(CuTest* tc, struct onk_test_state_t *ptr);
void CuTestDelete(CuTest *t);

/* Internal versions of assert functions -- use the public versions */
void CuFail_Line(CuTest* tc, const char* file, int line, const char* message2, const char* message);
void CuAssert_Line(CuTest* tc, const char* file, int line, const char* message, int condition);
void CuAssertStrEquals_LineMsg(CuTest* tc, 
	const char* file, int line, const char* message, 
	const char* expected, const char* actual);
void CuAssertIntEquals_LineMsg(CuTest* tc, 
	const char* file, int line, const char* message, 
	int expected, int actual);
void CuAssertDblEquals_LineMsg(CuTest* tc, 
	const char* file, int line, const char* message, 
	double expected, double actual, double delta);
void CuAssertPtrEquals_LineMsg(CuTest* tc, 
	const char* file, int line, const char* message, 
	void* expected, void* actual);

/* public assert functions */

#define CuFail(tc, ms)                        CuFail_Line(  (tc), __FILE__, __LINE__, NULL, (ms))
#define CuAssert(tc, ms, cond)                CuAssert_Line((tc), __FILE__, __LINE__, (ms), (cond))
#define CuAssertTrue(tc, cond)                CuAssert_Line((tc), __FILE__, __LINE__, "assert failed", (cond))

#define CuAssertStrEquals(tc,ex,ac)           CuAssertStrEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac))
#define CuAssertStrEquals_Msg(tc,ms,ex,ac)    CuAssertStrEquals_LineMsg((tc),__FILE__,__LINE__,(ms),(ex),(ac))
#define CuAssertIntEquals(tc,ex,ac)           CuAssertIntEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac))
#define CuAssertIntEquals_Msg(tc,ms,ex,ac)    CuAssertIntEquals_LineMsg((tc),__FILE__,__LINE__,(ms),(ex),(ac))
#define CuAssertDblEquals(tc,ex,ac,dl)        CuAssertDblEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac),(dl))
#define CuAssertDblEquals_Msg(tc,ms,ex,ac,dl) CuAssertDblEquals_LineMsg((tc),__FILE__,__LINE__,(ms),(ex),(ac),(dl))
#define CuAssertPtrEquals(tc,ex,ac)           CuAssertPtrEquals_LineMsg((tc),__FILE__,__LINE__,NULL,(ex),(ac))
#define CuAssertPtrEquals_Msg(tc,ms,ex,ac)    CuAssertPtrEquals_LineMsg((tc),__FILE__,__LINE__,(ms),(ex),(ac))

#define CuAssertPtrNotNull(tc,p)        CuAssert_Line((tc),__FILE__,__LINE__,"null pointer unexpected",(p != NULL))
#define CuAssertPtrNotNullMsg(tc,msg,p) CuAssert_Line((tc),__FILE__,__LINE__,(msg),(p != NULL))

/* CuSuite */
#define MAX_TEST_CASES	1024

/* stateless test */
#define SUITE_ADD_TEST(SUITE,TEST) CuSuiteAdd(SUITE, CuTestNew(#TEST, CuTestType, TEST))

/* reuse big buffers if possible */
/*  */
#define SUITE_ADD_STATE_TEST(SUITE, TEST) CuSuiteAdd(SUITE, CuTestNew(#TEST, ClonkTestType, TEST))

typedef struct
{
	int count;
	CuTest* list[MAX_TEST_CASES];
	int failCount;

} CuSuite;


void CuSuiteInit(CuSuite* testSuite);
CuSuite* CuSuiteNew(void);
void CuSuiteDelete(CuSuite *testSuite);
void CuSuiteAdd(CuSuite* testSuite, CuTest *testCase);
void CuSuiteAddSuite(CuSuite* testSuite, CuSuite* testSuite2);
void CuSuiteRun(CuSuite* testSuite, struct onk_test_state_t *ptr);
void CuSuiteSummary(CuSuite* testSuite, CuString* summary);
void CuSuiteDetails(CuSuite* testSuite, CuString* details);

#endif /* CU_TEST_H */
