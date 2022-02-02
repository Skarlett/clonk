
void __test__map_collection(CuTest* tc) {
    struct Token tokens[32];
    struct Parser state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
   
    static char * line[] = {
        "{1}", // expression, not collection
        "{1: 2}",
        "{1+2:{1:3+4}}",
        "{1:2, 3:4}",
        "{1:2+3, 3:4+5}",
        "{(1, 2, 3):(4, 5, 6), 3:4+5}",
    };
}