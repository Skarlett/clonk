use crate::tokens::TokenType;

pub const BUF_SZ: usize = 64;
pub type ExpectedBuf = [TokenType; BUF_SZ];

pub enum LexerError<'err_buf>
{
    Unexpected { expected: &'err_buf ExpectedBuf, got: TokenType },
    BadToken(logos::Span),
    InvalidState(String)
}

pub enum Error<'a> {
    UndefinedState,
    Lexer(LexerError<'a>),
}
