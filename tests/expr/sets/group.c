

void __test__effectively_empty_group(CuTest* tc) {
    struct onk_token_t tokens[32];
    struct Parser state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
    
    static char * src_code[] = {
        "()",
        "(())",
        "(())()",
        "(())(())",
        "(())[:]",
        "{}",
        "{{}}",
        "{{}}()",
    };

}