enum Opcode {
    ISANop,

    i64Const,
    i64Push,
    i64IsEq,
    i64Gt,
    i64Lt,
    i64GtEq,
    i64LtEq,
    i64Add,
    i64Sub,
    i64Div,
    i64Mul,
    i64Modolus,
    i64Pow,
    i64Square,
    // StrConst,
    // StrIsEq,
    StrCmp,

    Store,
    GStore,

    GLoad,
    Load,

    Goto,
    GotoT,
    GotoF,
    
    Call,
    
    SetFlag,

    Drop,
    Pop,
    
    OpenFD,


    Panic,

    Ret,
    Print
};