enum Opcode {
    ISANop,
    i32Const,
    
    i32Push,
    i32IsEq,
    i32Gt,
    i32Lt,
    i32GtEq,
    i32LtEq,

    i32Add,
    i32Sub,
    i32Div,
    i32Mul,
    i32Modolus,
    i32Pow,
    // short cuts
    i32Square,
    i32Inc,
    i32Dec,
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