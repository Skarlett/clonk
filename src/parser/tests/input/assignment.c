#include "lexer.h"
#include "common.h"

//#include "../../src/parser/error.h"

void __test__parse_simple_asn_expr(CuTest* tc) {
    struct onk_token_t tokens[32];
    struct onk_parser_state_t state;

    char str_buf[24];

    const enum onk_lexicon_t answers[] =
    {
      ONK_EQUAL_TOKEN, ONK_PLUSEQ_TOKEN, ONK_MINUS_EQL_TOKEN,
      ONK_BIT_NOT_EQL, ONK_BIT_AND_EQL, ONK_BIT_OR_EQL
    };

    const int nsymbols = 6;
    const char * symbols[] = {
      "=",
      "+=",
      "-=",
      "~=",
      "|=",
      "&="
    };


    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;


    for (int8_t i=0; nsymbols > i; i++)
    {
        sprintf((char *)str_buf, "x %s 10", symbols[i]);
        onk_tokenize(str_buf)
        answers[i]
    }
}
