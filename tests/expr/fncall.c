
// void __test__order_precedense_with_fncall(CuTest* tc) {
//     uint16_t ntokens, nqueue;
    
//     struct Token tokens[32],
//         *queue[32],
//         *masks[2];
    
//     static char * line = "first().middle.last()";
//     CuAssertTrue(tc, tokenize(line, tokens, &ntokens, NULL) == 0);
//     CuAssertTrue(tc, ntokens == 9);

//     postfix_expr(tokens, ntokens, queue, 32, masks, 2);
// }


void __test__fncall(CuTest* tc) {
    struct Token tokens[32];
    struct ExprParserState state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
    
    static char * line[] = {
        "foo()",
        "(foo())",
        "(foo)()()",
        "bar(1, 2)",
        "foo(1+2*3, 4)",
        "foo((1, 2))",
        "foo((1, 2), 3, 4)",
        "foo(bar(1, 2), 3, 4)",
        "foo(1, 2, 3, bar(4, 5))",
        "foo(bar(1, (2 * 2)), 3, 4)",

    };

    static enum Lexicon check_list[][16] = {
        {WORD, Apply, 0},
        {WORD, Apply, 0},
        {WORD, Apply, Apply, 0},

        {WORD, INTEGER, INTEGER, Apply, 0},
        {WORD, INTEGER, INTEGER, INTEGER, MUL, ADD, INTEGER, Apply, 0},
        {WORD, INTEGER, INTEGER, TupleGroup, Apply, 0},
        {WORD, INTEGER, INTEGER, TupleGroup, INTEGER, INTEGER, Apply, 0},
        {WORD, WORD, INTEGER, INTEGER, Apply, INTEGER, INTEGER, Apply, 0},
        {WORD, INTEGER, INTEGER, INTEGER, WORD, INTEGER, INTEGER, Apply, 0},
        {WORD, WORD, INTEGER, INTEGER, INTEGER, MUL, Apply, INTEGER, INTEGER, Apply, 0},        
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
        sprintf(msg, "failed on index %ld", i);
        AssertTokensByRef(tc, line[i], msg, state.debug.base, check_list[i]);
        reset_state(&state);
    }
}