cmake_minimum_required(VERSION 3.10)
project(clonk)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(CUTEST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libtest/CuTest/CuStr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libtest/CuTest/CuTest.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libtest/CuTest/suite.c
)

set(TESTLIB
    #src/libtest/build_mask.c
   ${CMAKE_CURRENT_SOURCE_DIR}/src/libtest/tokens.c
    #src/libtest/mask_assert.c
   ${CMAKE_CURRENT_SOURCE_DIR}/src/libtest/textutil.c
)

set(TESTLIB_TEST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libtest/test_assert.c
    #src/tests/libtest/test_mask.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libtest/CuTestTest.c
)

set(ONKSTD
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libstdonk/vec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libstdonk/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libstdonk/sort_merge.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libstdonk/int.c
)

set(ONKSTD_TEST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libstdonk/vec.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libstdonk/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libstdonk/merge_sort.c
)

set(PARSER
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libclonk/lexer/lexer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libclonk/lexer/helpers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libclonk/lexer/debug.c

    ${CMAKE_CURRENT_SOURCE_DIR}/src/libclonk/parser/error.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libclonk/parser/parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libclonk/parser/semantics.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/libclonk/parser/utils.c
)

set(PARSER_TEST
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libclonk/lexer/lexer_tests.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libclonk/lexer/harness.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libclonk/parser/parser.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libclonk/parser/semantics.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/libclonk/parser/predict.c
)

set(LIBONKCC
    ${ONKSTD}
    ${PARSER}
)

set(LIBONKCC_TEST
    ${CUTEST}

    ${TESTLIB}
    ${TESTLIB_TEST}

    ${ONKSTD}
    ${ONKSTD_TEST}

    ${PARSER}
    ${PARSER_TEST}
)

include_directories(include/)
add_executable("${PROJECT_NAME}-test" ${CMAKE_CURRENT_SOURCE_DIR}/src/tests/run_tests.c ${LIBONKCC_TEST})
add_executable("${PROJECT_NAME}-cc" ${CMAKE_CURRENT_SOURCE_DIR}/src/repl.c ${LIBONKCC})

install(TARGETS "${PROJECT_NAME}-cc")

# target_sources(test,
#     PRIVATE
#     "${LIBONKCC_TEST}"
#     "${LIBONKCC}"
#     PUBLIC
#     src/tests/run_tests.c
#     src/repl.c
# )
