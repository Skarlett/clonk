#include "parser.h"
#include "libtest/CuTest.h"

void __test__init_parser(CuTest* tc)
{
  struct onk_parser_state_t parser;
  uint16_t i=0;

  onk_parser_init(&parser, &i);

  // setups global namespace
  CuAssertTrue(tc, parser.operator_stack[0]->type == ONK_BRACE_OPEN_TOKEN);

  CuAssertTrue(tc, parser.set_stack[0].origin->type == ONK_BRACE_OPEN_TOKEN);

  CuAssertTrue(tc, parser.set_ctr == 1);
  CuAssertTrue(tc, parser.operators_ctr == 1);

  CuAssertTrue(tc, parser.operators_ctr == 1);
}

CuSuite * ParserSuite(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, __test__init_parser);

    return suite;
}
