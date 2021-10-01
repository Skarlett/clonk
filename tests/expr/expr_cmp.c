// /*
//    Prove incremential changes cause comparsion failure.
// */
// void __test__sanity_expr_cmp(CuTest* tc)
// {
//     struct Expr a, b;
    
//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     a.inner.uni.interal_data.symbol.tag = ValueTag;
//     a.inner.uni.interal_data.symbol.inner.value.type = IntT;
//     a.inner.uni.interal_data.symbol.inner.value.data.integer = 1;

//     CuAssertTrue(tc, cmpexpr(&a, &a) == 1);

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;
//     b.inner.uni.interal_data.symbol.tag = VariableTag;
//     b.inner.uni.interal_data.symbol.inner.variable = "a";
    
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     CuAssertTrue(tc, cmpexpr(&b, &b) == 1);

//     a.inner.uni.interal_data.symbol.tag = VariableTag;
//     a.inner.uni.interal_data.symbol.inner.variable = "b";

//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;

//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     a.inner.uni.interal_data.symbol.inner.variable = "a";
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
//     a.type = UndefinedExprT;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     b.type = UndefinedExprT;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
//     a.type = UniExprT;
//     b.type = UniExprT;


//     a.inner.uni.op = UniOpNop;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
    
//     a.inner.uni.op = UniValue;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

//     a.inner.uni.interal_data.symbol.tag = NullTag;
//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);

//     b.inner.uni.interal_data.symbol.tag = NullTag;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;

//     a.inner.uni.interal_data.symbol.tag = ValueTag;
//     b.inner.uni.interal_data.symbol.tag = ValueTag;

//     a.inner.uni.interal_data.symbol.inner.value.type = IntT;
//     b.inner.uni.interal_data.symbol.inner.value.type = IntT;
//     a.inner.uni.interal_data.symbol.inner.value.data.integer = 1;
//     b.inner.uni.interal_data.symbol.inner.value.data.integer = 2;

//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     b.inner.uni.interal_data.symbol.inner.value.data.integer = 1;
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);

//     b.type = UniExprT;
//     b.inner.uni.op = UniValue;

//     b.inner.uni.interal_data.symbol.inner.value.type = StringT;
//     b.inner.uni.interal_data.symbol.inner.value.data.string.capacity = 0;
//     b.inner.uni.interal_data.symbol.inner.value.data.string.length = 5;
//     b.inner.uni.interal_data.symbol.inner.value.data.string.ptr = "data\0";
    
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 0);
//     a.type = UniExprT;
//     a.inner.uni.op = UniValue;

//     a.inner.uni.interal_data.symbol.inner.value.type = StringT;
//     a.inner.uni.interal_data.symbol.inner.value.data.string.capacity = 0;
//     a.inner.uni.interal_data.symbol.inner.value.data.string.length = 5;
//     a.inner.uni.interal_data.symbol.inner.value.data.string.ptr = "data\0";
//     CuAssertTrue(tc, cmpexpr(&a, &b) == 1);
    
//     struct Expr bin;
//     bin.type = BinExprT;
//     bin.inner.bin.lhs = &a;
//     bin.inner.bin.rhs = &b;

//     CuAssertTrue(tc, cmpexpr(&bin, &bin) == 1);
//     CuAssertTrue(tc, cmpexpr(&a, &bin) == 0);

//     //TODO add test cases for fncalls
// }

