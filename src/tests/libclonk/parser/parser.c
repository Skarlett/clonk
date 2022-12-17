#include "parser.h"
#include "lexer.h"
#include "libtest/CuTest.h"
#include <string.h>

void __test__init_parser(CuTest* tc)
{
  struct onk_parser_state_t parser;
  struct onk_parser_input_t input;
  uint16_t i=0;

  memset(&input, 0, sizeof(struct onk_parser_input_t));
  onk_parser_init(&parser, &i);

  // setups global namespace
  CuAssertTrue(tc, parser.set_ctr == 0);
  CuAssertTrue(tc, parser.operators_ctr == 0);

  // setups global namespace
  onk_parser_construct(&parser, &i, &input);

  CuAssertTrue(tc, parser.operator_stack[0]->type == ONK_BRACE_OPEN_TOKEN);
  CuAssertTrue(tc, parser.set_stack[0].origin->type == ONK_BRACE_OPEN_TOKEN);
  CuAssertTrue(tc, parser.set_ctr == 1);
  CuAssertTrue(tc, parser.operators_ctr == 1);
}

void __test__reset_parser(CuTest* tc)
{
    struct onk_parser_state_t p1, p2;
    struct onk_parser_input_t parser_in;
    struct onk_parser_output_t parser_out;

    struct onk_lexer_input_t lex_in;
    struct onk_lexer_output_t lex_out;
    uint16_t i=0;

    onk_parser_init(&p1, &i);
    onk_parser_init(&p2, &i);
    CuAssertTrue(tc, onk_tokenize(&lex_in, &lex_out) == 0);
    // NOTE: add global scope ???? idk if this does anything atm
    onk_parser_input_from_lexer_output(
        &lex_out,
        &parser_in,
        true
    );
}

CuSuite * ParserSuite(void)
{
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, __test__init_parser);
    SUITE_ADD_TEST(suite, __test__reset_parser);
    return suite;
}
