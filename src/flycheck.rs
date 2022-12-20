use crate::TokenType;

type ExpectBuf = [TokenType; 256];

struct FlyCheck {
    expected: ExpectBuf,
    set_delim: bool,
    set_brace: bool
}

fn expect_expr(tok: TokenType, a: &mut ExpectBuf) {
    const CATCH: &'static [ &'static [TokenType]] = &[
    //        &[TokenType::Word, TokenType::BracketClose, TokenType::ParamClose],
            &[TokenType::String],
  //          &[TokenType::CloseBrace],
            &[TokenType::True, TokenType::False, TokenType::Integer]
    ];

    const SET: &'static [&'static [TokenType]] = &[
 //       &ASN_OPERATORS,
        &[TokenType::Dot],
        &[TokenType::BracketOpen, TokenType::ParamOpen],
//        &BIN_OPERATORS
    ];

    let mut i = 0;
    let mut exp_i = 0;

    for (rowi, &row) in CATCH.iter().enumerate()
    {
        for &item in row {
            if tok == item
            {
                i = rowi;
                break;
            }
        }
    }

    for row in &SET[i..]
    {
        for item in *row
        {
            a[exp_i] = *item;
            exp_i += 1;
        }
    }

}

fn expect_open_param(tok: TokenType) -> bool
{
    const EXPLICIT_OPEN_PARAM: &'static [TokenType] = &[
        TokenType::If,
        TokenType::While,
        TokenType::For
    ];

    EXPLICIT_OPEN_PARAM.contains(&tok)
}

fn expect_word(tok: TokenType) -> bool
{
    const EXPLICIT_WORD: &'static [TokenType] = &[
        TokenType::Struct,
        TokenType::Impl,
        // TokenType::Fn,
        TokenType::Dot
    ];

    EXPLICIT_WORD.contains(&tok)
}

// fn expect_expr(tok: TokenType) -> bool {
//     const EXPLICIT_EXPR: &'static [TokenType] = &[
//         TokenType::BracketOpen,
//         TokenType::ParamOpen,
//         TokenType::HashMapOpen,
//         _bin_ops(),
//         _uni_ops(),
//         _asn_ops(),
//         _delimiter()
//     ];

//     BIN_OPERATORS.contains(&tok);
//     ASN_OPERATORS.contains(x)


//     return EXPLICIT_EXPR.contains(&tok)
// }
