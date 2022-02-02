
void __test__simple_order_precedence(CuTest* tc) {
    struct Token tokens[32];
    struct Parser state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
   
    static char * line[] = {
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

    static enum Lexicon check_list[][16] = {
        {INTEGER, INTEGER, ADD, INTEGER, MUL, 0},
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, INTEGER, MUL, ADD, 0},
        {INTEGER, INTEGER, DIV, INTEGER, ADD, 0},
        {WORD, WORD, ADD, WORD, WORD, MUL, SUB, 0},
        {INTEGER, INTEGER, MUL, INTEGER, ADD, 0},
        {INTEGER, INTEGER, INTEGER, MUL, ADD, 0},
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, ADD, INTEGER, MUL, 0},
        {INTEGER, INTEGER, INTEGER, ADD, DIV, 0},
        {WORD, WORD, WORD, SUB, WORD, MUL, ADD, 0},
        {INTEGER, INTEGER, INTEGER, ADD, MUL, 0},
        {WORD, WORD, DOT, INTEGER, ADD, 0},
        {INTEGER, WORD, WORD, DOT, ADD, 0},
        {WORD, WORD, DOT, WORD, DOT, INTEGER, ADD, 0},
        {WORD, INTEGER, EQUAL, 0},
        {WORD, WORD, WORD, EQUAL, EQUAL, 0},
        {WORD, WORD, WORD, EQUAL, EQUAL, 0},
        {WORD, WORD, INTEGER, WORD, MUL, EQUAL, EQUAL, 0},
        {WORD, WORD, INTEGER, MUL, WORD, EQUAL, EQUAL, 0},
        {WORD, WORD, DOT, WORD, DOT, WORD, WORD, DOT, INTEGER, MUL, EQUAL,  0},
        0
    };

    for (uint16_t i=0 ;; i++) {
        if (check_list[i][0] == 0)
            break;
        ntokens=0;
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "%s:%d failed tokenizing", __FILE__, __LINE__);
        CuAssert(tc, msg, tokenize(line[i], tokens, &ntokens, 32, false, NULL) == 0);
        
        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on parsing expr (idx): %d", i);
        CuAssert(tc, msg, parse_expr(line[i], tokens, ntokens, &state, ret) == 0); 

        memset(msg, 0, sizeof(char[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ]));
        sprintf(msg, "failed on index %d", i);
        AssertTokensByRef(tc, line[i], msg, state.debug.base, check_list[i]);
        reset_state(&state);
    }
}