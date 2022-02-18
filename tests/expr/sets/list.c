

void __test__list_collection(CuTest* tc) {
    struct onk_token_t tokens[32];
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
        {onk_tuple_group_token, 0},
        {ONK_INTEGER_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_ADD_TOKEN, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_tuple_group_token, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_tuple_group_token, onk_tuple_group_token, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN,  onk_tuple_group_token, ONK_INTEGER_TOKEN, onk_tuple_group_token, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_tuple_group_token, 0},
        {ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, ONK_INTEGER_TOKEN, onk_tuple_group_token, 0},
        0
    };
}

