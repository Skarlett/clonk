
void __test__simple_order_precedence(CuTest* tc) {
    struct onk_token_t tokens[32];
    struct Parser state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
   
    static char * src_code[] = {
        "(1 + 3) * 4",
        "1 + 2",
        "1 + 3 * 4",
        "1 / 2 + 2",
        "a + b - c * d",
        "1 * 2 + 3",
        "1 + 2 * 3",
        "(1 + 2)",
        "((1 + 2))",
        "(1 + 3) * 4",
        "1 / (2 + 2)",
        "a + (b - c) * d",
        "1 * (2 + 3)",
        "foo.attr + 4",
        "4 + foo.attr",
        "foo.bar.attr + 2",
        "2 + foo.bar.attr",
        "x = 2",
        "x = (z = y)",
        "x = y = z",
        "x = (y = 2 * x)",
        "x = y * 2",
        "x.y.z = a.b * 2",
        0
    };

    static enum onk_lexicon_t check_list[][16] = {
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_DIV_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_ADD_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_MUL_TOKEN, ONK_SUB_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_DIV_TOKEN, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_SUB_TOKEN, ONK_WORD_TOKEN, ONK_MUL_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, ONK_MUL_TOKEN, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_DOT_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_DOT_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_DOT_TOKEN, ONK_WORD_TOKEN, ONK_DOT_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, EQUAL, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, EQUAL, EQUAL, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, EQUAL, EQUAL, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_WORD_TOKEN, ONK_MUL_TOKEN, EQUAL, EQUAL, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, ONK_WORD_TOKEN, EQUAL, EQUAL, 0},
        {ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_DOT_TOKEN, ONK_WORD_TOKEN, ONK_DOT_TOKEN, ONK_WORD_TOKEN, ONK_WORD_TOKEN, ONK_DOT_TOKEN, ONK_INTEGER_TOKEN, ONK_MUL_TOKEN, EQUAL,  0},
        0
    };

    for (uint16_t i=0 ;; i++) {
        if (check_list[i][0] == 0)
            break;
        ntokens=0;
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        CuAssert(tc, msg, onk_tokenize(src_code[i], tokens, &ntokens, 32, false, NULL) == 0);
        
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on parsing expr (idx): %d", i);
        CuAssert(tc, msg, parse_expr(src_code[i], tokens, ntokens, &state, ret) == 0); 

        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on index %d", i);
        onk_assert_tokens_by_ref(tc, src_code[i], msg, state.debug.base, check_list[i]);
        parser_reset(&state);
    }
}