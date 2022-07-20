#include "libtest/CuTest.h"
#include "clonk.h"
#include "lexer.h"
#include "parser.h"
#include "predict.h"
#include "onkstd/merge_sort.h"

/***************************/
/*         utils           */
/**************************/
void compile(struct onk_parser_state_t *parser)
{
    struct validator_frame_t frame;

    _onk_semantic_next_frame(&frame, parser);
    parser->nexpect = _onk_semantic_compile(
        parser->expect, &frame
    );
}

void mock_current(struct onk_parser_state_t *parser, enum onk_lexicon_t type) {
    struct onk_token_t token;
    token.type = type;

    parser->src = &token;
    parser->src_sz = 1;

    compile(parser);
}

void assert_expect(enum onk_lexicon_t *a, enum onk_lexicon_t *b, uint16_t len)
{
  onk_merge_sort_u16((void *)a, 0, len);
}

/********************************/
/* Jump to explicit param open  */
/********************************/

void __test__semantic_param_after_while(CuTest* tc, struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_WHILE_TOKEN);
    assert(state->parser.nexpect == 1);
    assert(state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);
}

void __test__semantic_param_after_if(CuTest* tc, struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_IF_TOKEN);
    CuAssertTrue(tc, state->parser.nexpect == 1);
    CuAssertTrue(tc, state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);
}

void __test__semantic_param_after_for(CuTest* tc, struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_FOR_TOKEN);
    CuAssertTrue(tc, state->parser.nexpect == 1);
    CuAssertTrue(tc, state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);
}

/********************************/
/*     Jump to explicit word    */
/********************************/
void __test__semantic_word_after_dot(
    CuTest* tc,
    struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_DOT_TOKEN);
    assert(state->parser.nexpect == 1);
    assert(state->parser.expect[0] == ONK_WORD_TOKEN);
}

void __test__semantic_word_after_struct(
    CuTest* tc,
    struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_STRUCT_TOKEN);
    assert(state->parser.nexpect == 1);
    assert(state->parser.expect[0] == ONK_WORD_TOKEN);
}

void __test__semantic_word_after_impl(
    CuTest* tc,
    struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_IMPL_TOKEN);
    assert(state->parser.nexpect == 1);
    assert(state->parser.expect[0] == ONK_WORD_TOKEN);
}

void __test__semantic_word_after_import(
    CuTest* tc,
    struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_IMPORT_TOKEN);
    assert(state->parser.nexpect == 1);
    assert(state->parser.expect[0] == ONK_WORD_TOKEN);
}

/* void __test__semantic_word_after_from( */
/*     CuTest* tc, */
/*     struct onk_test_state_t *state */
/* ){ */
/*     mock_current(&state->parser, ONK_FROM_TOKEN); */
/*     assert(state->parser.nexpect == 1); */
/*     assert(state->parser.expect[0] == ONK_WORD_TOKEN); */
/* } */

/********************************/
/*   Test fall-through table    */
/********************************/
void __test__semantic_operator_after_unit(
    CuTest* tc,
    struct onk_test_state_t *state
){
    const uint8_t set_len = BINOP_LEN \
      + ASNOP_LEN \
      + DELIM_LEN \
      + UNIOP_LEN \
      + 3;

    const enum onk_lexicon_t set[] = {
        _EX_BIN_OPERATOR,
        _EX_ASN_OPERATOR,
        _EX_DELIM,
        _EX_UNARY_OPERATOR,
        ONK_HASHMAP_LITERAL_START_TOKEN,
        ONK_BRACKET_OPEN_TOKEN,
        ONK_PARAM_OPEN_TOKEN,
    };

    const uint8_t answer_len = UNIT_LEN + UNIOP_LEN + 3;
    const enum onk_lexicon_t answer[] = {
      _EX_UNARY_OPERATOR,
      _EX_UNIT,
      ONK_PARAM_OPEN_TOKEN,
      ONK_BRACKET_OPEN_TOKEN,
      ONK_HASHMAP_LITERAL_START_TOKEN
    };

    onk_merge_sort_u16((void*)answer, 0, answer_len);

    for(uint8_t i=0; set_len > i; i++)
    {
        mock_current(&state->parser, set[i]);
        compile(&state->parser);
        CuAssertTrue(tc, state->parser.nexpect == answer_len);

        onk_merge_sort_u16((void*)state->parser.expect, 0, answer_len);
        CuAssertTrue(tc, memcmp(state->parser.expect, answer, sizeof(enum onk_lexicon_t) * answer_len));
    }
}

void __test__semantic_unit_after_operator(
    CuTest* tc,
    struct onk_test_state_t *state
){
    const uint8_t answer_len = UNIT_LEN + UNIOP_LEN + 3;
    const enum onk_lexicon_t anwser[] = {
      _EX_UNARY_OPERATOR,
      _EX_UNIT,
      ONK_PARAM_OPEN_TOKEN,
      ONK_BRACKET_OPEN_TOKEN,
      ONK_HASHMAP_LITERAL_START_TOKEN
    };

    const uint8_t set_len = BINOP_LEN \
      + ASNOP_LEN \
      + DELIM_LEN \
      + UNIOP_LEN \
      + 3;

    const enum onk_lexicon_t set[] = {
        _EX_BIN_OPERATOR,
        _EX_ASN_OPERATOR,
        _EX_DELIM,
        _EX_UNARY_OPERATOR,
        ONK_HASHMAP_LITERAL_START_TOKEN,
        ONK_BRACKET_OPEN_TOKEN,
        ONK_PARAM_OPEN_TOKEN,
    };

    for(uint8_t i=0; set_len > i; i++)
    {
      mock_current(&state->parser, set[i]);
      for(uint8_t j=0; answer_len > j; j++)
          CuAssertTrue(tc, onk_lexarr_contains(
              state->parser.expect[j],
              (void *)anwser,
              answer_len
          ) == 1);
    }

    CuAssertTrue(tc, state->parser.nexpect == 1);
    CuAssertTrue(tc, state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);

}

void __test__semantic_after_add(CuTest* tc, struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_ADD_TOKEN);
    CuAssertTrue(tc, state->parser.nexpect == 1);
    CuAssertTrue(tc, state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);
}

void __test__delim_list(CuTest* tc, struct onk_test_state_t *state)
{
    struct validator_frame_t frame;
    _onk_semantic_next_frame(&frame, &state->parser);

    mock_current(&state->parser, ONK_INTEGER_TOKEN);

    CuAssertTrue(tc, state->parser.nexpect == 1);
    CuAssertTrue(tc, state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);
}

void __test__delim_tuple(CuTest* tc, struct onk_test_state_t *state)
{
    struct validator_frame_t frame;
    _onk_semantic_next_frame(&frame, &state->parser);

    mock_current(&state->parser, INTEGER);

    CuAssertTrue(tc, state->parser.nexpect == 1);
    CuAssertTrue(tc, state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);
}




CuSuite * Semantic(void)
{
    CuSuite* suite = CuSuiteNew();

    SUITE_ADD_TEST(suite, __test__semantic_param_after_for);
    SUITE_ADD_TEST(suite, __test__semantic_param_after_if);
    SUITE_ADD_TEST(suite, __test__semantic_param_after_while);

    SUITE_ADD_TEST(suite, __test__semantic_word_after_dot);
    SUITE_ADD_TEST(suite, __test__semantic_word_after_struct);
    SUITE_ADD_TEST(suite, __test__semantic_word_after_impl);
    SUITE_ADD_TEST(suite, __test__semantic_word_after_import);

    SUITE_ADD_TEST(suite, __test__semantic_operator_after_unit);
    SUITE_ADD_TEST(suite, __test__semantic_unit_after_operator);


    return suite;
}
