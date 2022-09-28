cmake_minimum_required(VERSION 3.10)
project(clonk)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(CUTEST
    src/libtest/CuTest/CuStr.c
    src/libtest/CuTest/CuTest.c
    src/libtest/CuTest/suite.c
)

set(TESTLIB
    src/libtest/build_mask.c
    src/libtest/tokens.c
    src/libtest/mask_assert.c
    src/libtest/textutil.c
)

set(TESTLIB_TEST
    src/tests/libtest/test_assert.c
    src/tests/libtest/test_mask.c
    src/tests/libtest/CuTestTest.c
)

set(ONKSTD
    src/libstdonk/vec.c
    src/libstdonk/queue.c
    src/libstdonk/sort_merge.c
    src/libstdonk/int.c
)

set(ONKSTD_TEST
    src/tests/libstdonk/vec.c
    src/tests/libstdonk/queue.c
    src/tests/libstdonk/merge_sort.c
)

set(PARSER
    src/libclonk/lexer/lexer.c
    src/libclonk/lexer/helpers.c
    src/libclonk/lexer/debug.c

    src/libclonk/parser/error.c
    src/libclonk/parser/parser.c
    src/libclonk/parser/semantics.c
    src/libclonk/parser/semantic_utils.c
    src/libclonk/parser/utils.c
)

set(PARSER_TEST
    src/tests/libclonk/lexer/lexer_tests.c
    src/tests/libclonk/lexer/harness.c
    src/tests/libclonk/parser/parser.c
    src/tests/libclonk/parser/semantics.c
    src/tests/libclonk/parser/predict.c
)

set(ONKCC
    "${ONKSTD}"
    "${PARSER}"
)

set(ONKCC_TEST
    "${ONKCC}"
    "${CUTEST}"
    "${TESTLIB}"
    "${TESTLIB_TEST}"
    "${ONKSTD_TEST}"
    "${PARSER_TEST}"
)

include_directories(include/ )
# add_executable(cclonk src/main.c "${ONKCC}")

add_executable(cclonk src/repl.c "${ONKCC}")
add_executable(test src/tests/run_tests.c "${ONKCC_TEST}")
