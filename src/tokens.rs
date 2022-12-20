#![feature(const_fn)]

use logos::{Lexer, Logos};
use std::borrow::Cow;
use std::ops::Range;

use crate::errors::{ExpectedBuf, LexerError};

#[derive(Debug, Clone)]
pub struct Token {
    pub loc: Range<usize>,
    pub ttype: TokenType,
}

impl Token {
    pub fn new(ttype: TokenType, rng: Range<usize>) -> Self {
        Token { ttype, loc: rng }
    }
}


/*
 * Do not change the ordering of this structure.
 * Lexer is dependent upon it.
*/
#[derive(Logos, Debug, PartialEq, Clone, Copy, Eq, PartialOrd, Ord)]
pub enum TokenType {
    EOF,

    #[error]
    Undefined,

    #[regex(r"\#+\n")]
    Comment,

    #[regex(r"[ \t\n\f]+", logos::skip)]
    Whitespace,

    #[token("in")]
    In,

    #[token("struct")]
    Struct,

    #[token("impl")]
    Impl,

    #[token("return")]
    Return,

    #[token("import")]
    Import,

    #[token("break")]
    Break,

    #[token("continue")]
    Continue,

    #[token("from")]
    From,

    #[token("if")]
    If,

    #[token("else")]
    Else,

    #[token("def")]
    Fn,

    #[token("for")]
    For,

    #[token("while")]
    While,

    #[token("true")]
    True,

    #[token("false")]
    False,

    #[token(">>")]
    ShiftRight,

    #[token("<<")]
    ShiftLeft,

    #[token("or")]
    #[token("||")]
    Or,

    #[token("&&")]
    #[token("and")]
    And,

    #[token(">=")]
    GreaterThanEql,

    #[token("<=")]
    LesserThanEql,

    #[token("==")]
    IsEql,

    #[token("!=")]
    IsNotEql,

    #[token("|=")]
    BitOrEql,

    #[token("&=")]
    BitAndEql,

    #[token("~=")]
    BitNotEql,

    #[token("+=")]
    AddEql,

    #[token("-=")]
    SubEql,

    #[token("=")]
    Eql,

    #[token("+")]
    Add,

    #[token("-")]
    Sub,

    #[token("|")]
    Pipe,

    #[token("&")]
    Amper,

    #[token(">")]
    GreaterThan,

    #[token("<")]
    LesserThan,

    #[token(",")]
    Comma,

    #[token(":")]
    Colon,

    #[token(";")]
    Semicolon,

    #[token("]")]
    BracketClose,

    #[token("}")]
    BraceClose,

    #[token(")")]
    ParamClose,

    #[token("[")]
    BracketOpen,

    #[token("${")]
    MapBrace,

    #[token("{")]
    BraceOpen,

    #[token("(")]
    ParamOpen,

    #[token("%")]
    Mod,

    #[token(".")]
    Dot,

    #[token("*")]
    Mul,

    #[token("/")]
    Div,

    #[token("^")]
    Pow,

    #[token("~")]
    Tilde,

    #[token("!")]
    Not,

    #[token("\"")]
    DoubleQuote,

    #[token("'")]
    SingleQuote,

    #[token("$")]
    #[token("£")]
    #[token("₤")]
    Dollar,

    #[token("\\")]
    Backslash,

    #[regex("-?[0-9][0-9_]+")]
    Integer,

    #[regex("-?[0-9]+\\.[0-9_]+")]
    Float,

    #[regex(r#""([^"\\]|\\t|\\u|\\n|\\")*""#)]
    String,

    #[regex("[a-zA-Z_][a-zA-Z0-9_]+")]
    Ident,
}

#[derive(Copy, Debug, Clone, PartialEq, Ord, PartialOrd, Eq)]
enum GroupIdentity {
    Tuple,
    List,
    Map,
    Subscript,
    Codeblock,
}

#[derive(Copy, Debug, Clone, PartialEq, Ord, PartialOrd, Eq)]
pub enum ParseProduct {
    IfCond,
    IfBody,
    WhileCond,
    WhileBody,
    ForArgs,
    ForBody,

    FnBody,
    FnSig,

    Apply,
    Index,

    Group { member_len: usize, identity: GroupIdentity }
}

const ASN_OPERATORS: &'static [TokenType] = &[
    TokenType::AddEql,
    TokenType::SubEql,
    TokenType::Eql,
    TokenType::BitOrEql,
    TokenType::BitNotEql,
    TokenType::BitAndEql,
];

const BIN_OPERATORS: &'static [TokenType] = &[
    TokenType::Add,
    TokenType::Sub,
    TokenType::Div,
    TokenType::Mul,
    TokenType::Pow,
    TokenType::Mod,
    TokenType::Amper,
    TokenType::Pipe,
    TokenType::Dot,
];

fn strip_underscore<'a>(lex: &mut Lexer<'a, TokenType>) -> Cow<'a, str> {
    lex.slice().chars().filter(|&c| c != '_').collect()
}

impl TokenType {
    pub fn is_unit(&self) -> bool {
        const UNIT: &'static [TokenType] = &[
            TokenType::Ident,
            TokenType::Integer,
            TokenType::String,
            TokenType::True,
            TokenType::False
        ];
        UNIT.contains(self)
    }

    pub fn is_bin_operator(&self) -> bool
    { BIN_OPERATORS.contains(self) }

    pub fn is_assign_operator(&self) -> bool
    { ASN_OPERATORS.contains(self) }

    pub fn is_unary_operator(&self) -> bool {
        [TokenType::Tilde, TokenType::Not].contains(self)
    }

    pub fn is_open_brace(&self) -> bool {
        [
            TokenType::MapBrace,
            TokenType::BraceOpen,
            TokenType::BracketOpen,
            TokenType::ParamOpen
        ].contains(self)
    }

    pub fn is_close_brace(&self) -> bool {
        [
            TokenType::BraceClose,
            TokenType::BracketClose,
            TokenType::ParamClose
        ].contains(self)
    }

    pub fn is_delimiter(&self) -> bool {
        [
            TokenType::Semicolon,
            TokenType::Colon,
            TokenType::Comma
        ].contains(self)
    }

    pub fn invert_brace(&self) -> TokenType {
        #[cfg(debug_assertions)]
        assert!(self.is_close_brace() || self.is_open_brace());


        return TokenType::ERROR;
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn parser_operator() {
        let answer: &[TokenType] = &[
            TokenType::LesserThanEql,
            TokenType::GreaterThanEql,
            TokenType::IsEql,
            TokenType::IsNotEql,
            TokenType::AddEql,
            TokenType::SubEql,
            TokenType::And,
            TokenType::Or,
            TokenType::ShiftRight,
            TokenType::ShiftLeft,
            TokenType::BitOrEql,
            TokenType::BitAndEql,
            TokenType::BitNotEql,
            TokenType::LesserThanEql,
            TokenType::Eql,
            TokenType::GreaterThanEql,
            TokenType::Eql,
            TokenType::IsEql,
            TokenType::LesserThan,
            TokenType::IsEql,
            TokenType::GreaterThan,
            TokenType::GreaterThanEql,
            TokenType::Eql,
            TokenType::LesserThanEql,
            TokenType::Eql,
            TokenType::IsEql,
            TokenType::Eql,
            TokenType::IsNotEql,
            TokenType::Eql,
            TokenType::IsEql,
            TokenType::Not,
            TokenType::Add,
            TokenType::AddEql,
            TokenType::Sub,
            TokenType::SubEql,
            TokenType::Eql,
            TokenType::Add,
            TokenType::Add,
            TokenType::Eql,
            TokenType::Sub,
            TokenType::Sub,
            TokenType::AddEql,
            TokenType::Eql,
            TokenType::SubEql,
            TokenType::Eql,
            TokenType::IsEql,
            TokenType::Add,
            TokenType::IsEql,
            TokenType::Sub,
            TokenType::And,
            TokenType::Amper,
            TokenType::Or,
            TokenType::Pipe,
            TokenType::ShiftRight,
            TokenType::GreaterThan,
            TokenType::ShiftRight,
            TokenType::Eql,
            TokenType::ShiftLeft,
            TokenType::Eql,
            TokenType::ShiftLeft,
            TokenType::LesserThan,
            TokenType::BitOrEql,
            TokenType::Eql, //TokenType::Or,
                            //TokenType::GreaterThan,
        ];

        let src = r#"<= >= == != += -= && || >>
            << |= &= ~= <== >== ==< ==>
            >== <== === !== ==! ++= --=
            =++ =-- +== -== ==+ ==- &&&
            ||| >>> >>= <<= <<< |==
            "#;

        let lexer = TokenType::lexer(src);

        for (i, tok) in lexer.enumerate() {
            assert_eq!(tok, answer[i]);
        }
    }

    #[test]
    fn integer() {
        let src = "32_212 _3211";

        let mut lexer = TokenType::lexer(src).peekable();
        assert_eq!(lexer.next(), Some(TokenType::Integer));
        assert_eq!(lexer.next(), Some(TokenType::Ident));
        assert_eq!(lexer.next(), None);
    }
}
