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
    // short cuts
    i64Square,
    i64Inc,
    i64Dec,
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
    
    Drop,
    Pop,
    
    Halt,

    Ret,
    Print
};