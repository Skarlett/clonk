

void __test__list_collection(CuTest* tc) {
    struct Token tokens[32];
    struct Parser state;
    struct Expr *ret;

    char msg[__SIM_ORD_PRECEDENSE_MSG_BUF_SZ];
    uint16_t ntokens=0;
   
    static char * src_code[] = {
        "[]",
        "[1]",
        "[1, 2]",
        "[1, 2+3]",
        "[1, [2+3]]",
        "[[1, 2+3]]",
        "[1, [2+3]]",  
        "[1, 2+3]+4",      
        "[][2]",
        "[][]",
        "[[]]",
    };

    static enum onk_lexicon_t check_list[][8] = {
        {TupleGroup, 0},
        {INTEGER, 0},
        {INTEGER, INTEGER, ADD, 0},
        {INTEGER, INTEGER, TupleGroup, 0},
        {INTEGER, INTEGER, INTEGER, TupleGroup, TupleGroup, 0},
        {INTEGER, INTEGER,  TupleGroup, INTEGER, TupleGroup, 0},
        {INTEGER, INTEGER, INTEGER, TupleGroup, 0},
        {INTEGER, INTEGER, INTEGER, TupleGroup, 0},
        0
    };
}

