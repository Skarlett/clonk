
void __test__code_block(CuTest* tc) {
    struct Token tokens[32];
    struct Parser state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
   
    static char * line[] = {
        "{}",
        "{1}", // expression, not collection
        "{1}", // expression, not collection
        "{ foo(); }",
        "a = {1; 2; 3;};",
    };
}
