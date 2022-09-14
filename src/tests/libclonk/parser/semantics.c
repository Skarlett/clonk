#include "libtest/CuTest.h"
#include "clonk.h"
#include "lexer.h"
#include "parser.h"
#include "semantics.h"
#include "onkstd/merge_sort.h"

/***************************/
/*         utils           */
/**************************/
struct onk_token_t mk_tok(enum onk_lexicon_t token) {
    struct onk_token_t ret;

    memset(&ret, 0, sizeof(struct onk_token_t));
    ret.type = token;
    return ret;
}

void compile(struct validator_frame_t *frame, struct onk_parser_state_t *parser)
{
    onk_semantic_next_frame(frame, parser);

    parser->nexpect = onk_semantic_compile(
        parser->expect, frame
    );
}

void mock_current(struct onk_parser_state_t *parser, enum onk_lexicon_t type)
{
    struct onk_token_t token;
    token.type = type;

    parser->src = &token;
    parser->src_sz = 1;
}

/********************************/
/* Jump to explicit param open  */
/********************************/


// while(
//      ^
void __test__semantic_param_after_while(CuTest* tc, struct onk_test_state_t *state)
{
    struct validator_frame_t frame;

    mock_current(&state->parser, ONK_WHILE_TOKEN);
    compile(&state->parser);

    CuAssertTrue(tc, state->parser.nexpect == 1);
    CuAssertTrue(tc, state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);
}

// if(
//   ^
void __test__semantic_param_after_if(CuTest* tc, struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_IF_TOKEN);
    compile(&state->parser);

    CuAssertTrue(tc, state->parser.nexpect == 1);
    CuAssertTrue(tc, state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);
}

// for(
//    ^
void __test__semantic_param_after_for(CuTest* tc, struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_FOR_TOKEN);
    compile(&state->parser);

    CuAssertTrue(tc, state->parser.nexpect == 1);
    CuAssertTrue(tc, state->parser.expect[0] == ONK_PARAM_OPEN_TOKEN);
}

/********************************/
/*     Jump to explicit word    */
/********************************/

// word.word
//     ^^
void __test__semantic_word_after_dot(
    CuTest* tc,
    struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_DOT_TOKEN);
    compile(&state->parser);

    assert(state->parser.nexpect == 1);
    assert(state->parser.expect[0] == ONK_WORD_TOKEN);
}

// struct word
//      ^-^
void __test__semantic_word_after_struct(
    CuTest* tc,
    struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_STRUCT_TOKEN);
    compile(&state->parser);

    assert(state->parser.nexpect == 1);
    assert(state->parser.expect[0] == ONK_WORD_TOKEN);
}

// struct word
//      ^-^
void __test__semantic_word_after_impl(
    CuTest* tc,
    struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_IMPL_TOKEN);
    compile(&state->parser);

    assert(state->parser.nexpect == 1);
    assert(state->parser.expect[0] == ONK_WORD_TOKEN);
}

// import word
//      ^-^
void __test__semantic_word_after_import(
    CuTest* tc,
    struct onk_test_state_t *state)
{
    mock_current(&state->parser, ONK_IMPORT_TOKEN);
    compile(&state->parser);

    assert(state->parser.nexpect == 1);
    assert(state->parser.expect[0] == ONK_WORD_TOKEN);
}

/********************************/
/*         delim placement      */
/********************************/

// import word
//      ^-^
void __test__semantic_delim_kw(
    CuTest* tc,
    struct onk_test_state_t *state)
{
    struct onk_token_t open_param = mk_tok(ONK_PARAM_OPEN_TOKEN), gmod;
    const enum onk_lexicon_t GROUP_OPS[] = {
        onk_ifcond_op_token,
        onk_while_cond_op_token,
    };
    const uint8_t GROUP_OPS_LEN = 2;

    state->parser.operator_stack[1] = &gmod;
    state->parser.operator_stack[2] = &open_param;

    mock_current(&state->parser, ONK_WORD_TOKEN);

    for (uint8_t i=0; GROUP_OPS_LEN > i; i++)
    {
        memset(&gmod, 0, sizeof(struct onk_token_t));
        gmod.type = GROUP_OPS[i];

        compile(&state->parser);

        CuAssertTrue(tc, onk_lexarr_contains(ONK_COMMA_TOKEN, state->parser.expect, state->parser.nexpect) == false);
        CuAssertTrue(tc, onk_lexarr_contains(ONK_COLON_TOKEN, state->parser.expect, state->parser.nexpect) == false);
        CuAssertTrue(tc, onk_lexarr_contains(ONK_SEMICOLON_TOKEN, state->parser.expect, state->parser.nexpect) == false);
    }

    assert(state->parser.nexpect == 1);
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
// unit +
//    ^-^
void __test__semantic_operator_after_unit(
    CuTest* tc,
    struct onk_test_state_t *state
){
    char buf[254];
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

        snprintf(buf, 254, "FAILED equality check\ni: %d", i);
        onk_merge_sort_u16((void*)state->parser.expect, 0, answer_len);

        CuAssert(tc, buf, state->parser.nexpect == answer_len);
        CuAssert(tc, buf, memcmp(state->parser.expect, answer, sizeof(enum onk_lexicon_t) * answer_len));
    }
}

// .. + unit
//    ^-^
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
}

/********************************/
/*        Group delimiter       */
/********************************/
void __test__delim_list(CuTest* tc, struct onk_test_state_t *state)
{
    struct validator_frame_t frame;
    struct onk_token_t open_brace = mk_tok(ONK_PARAM_OPEN_TOKEN);

    state->parser.operator_stack[1] = &open_brace;
    state->parser.set_stack[1].origin = &open_brace;

    mock_current(&state->parser, ONK_WORD_TOKEN);
    onk_semantic_next_frame(&frame, &state->parser);

    compile(&frame);
}

void __test__delim_tuple(CuTest* tc, struct onk_test_state_t *state)
{
    struct validator_frame_t frame;
    onk_semantic_next_frame(&frame, &state->parser);

 //   mock_current(&state->parser, ONK_INTEGER_TOKEN);
}

void __test__group_tuple(CuTest* tc, struct onk_test_state_t *state)
{
    struct validator_frame_t frame;
    onk_semantic_next_frame(&frame, &state->parser);

 //   mock_current(&state->parser, ONK_INTEGER_TOKEN);

}

/********************************/
/*        Group context         */
/********************************/
void __test__ctx_index_op(CuTest* tc, struct onk_test_state_t *state)
{
    struct validator_frame_t frame;

    onk_semantic_next_frame(&frame, &state->parser);

    //mock_current(&state->parser, ONK_INTEGER_TOKEN);

}

CuSuite * SemanticSuite(void)
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
