#[macro_use]
extern crate smallvec;

use std::iter::Peekable;
use smallvec::{SmallVec, smallvec};
use logos::{Logos, Lexer};

pub enum Error {
    Lexer(LexerError),
}

pub enum LexerError {
    Unexpected { expected: SmallVec<[Token; 8]>, got: Token },
    BadToken(logos::Span)
}

impl<'src> Lexicon
where Self: Logos<'src>
{
    type Lexer = Lexer<'src, Self>;

    fn build<F>(
        lexer: &mut Lexer<'src, Self>,
    )

    fn identifier(
        lexer: &mut Lexer<'src, Self>,
        seq: u16
    ) -> Result<Self, LexerError>

    {
        unimplemented!()

        // let start = lexer.span().start;
        // let mut end = start.clone();
        // let mut prev_token = Token::Undefined;

        // let mut iter = lexer.peekable().enumerate();
        // while let Some((i, tok)) = iter.enumerate() {
        //     match (prev_token, tok) {
        //         (Token::Underscore, Token::Digit) if 2 > i => {
        //             return Err(LexerError::Unexpected {
        //                 expected: unimplemented!(), //smallvec![Token::Digit, Token::Char],
        //                 got: Token::Underscore
        //             })
        //         }

        //         (_, Token::Char
        //          | Token::Underscore
        //          | Token::Digit) => continue,

        //         _ => {
        //             end = lexer.span();
        //             break;
        //         }
        //     }

        //     prev_token = tok;
        // }


        // Ok(ParsedToken {
        //     token: Token::Integer,
        //     slice: start..end,
        //     seq
        // })

    }

    fn integer(lexer: &mut Lexer<'src, Self>) -> Result<Self, LexerError>
    {
        unimplemented!()
        // let start = lexer.span().start;
        // let mut end = start.clone();
        // let mut prev_token = Token::Undefined;

        // while let Some(tok) = lexer.next() {
        //     match (prev_token, tok)
        //     {
        //         (Token::Underscore, Token::Underscore) =>
        //             return Err(LexerError::Unexpected {
        //                 got: Token::Underscore,
        //                 expected: smallvec![Token::Digit]
        //             }),

        //         (_, Token::Digit | Token::Underscore) =>
        //             continue,

        //         _ => {
        //             end = lexer.span();
        //             break;
        //         }
        //     }

        //     prev_token = tok;
        // }


        // Ok(ParsedToken {
        //     token: Token::Integer,
        //     slice: start..end,
        //     seq
        // })
    }

    fn string(
        lexer: &mut Lexer<'src, Self>,
        quote: Token,
    ) -> Result<Self, LexerError>
    {
        unimplemented!()

        // let start = lexer.span().start;
        // let mut end = start.clone();
        // let mut prev_token = Token::UNDEFINED;

        // while let Some(tok) = lexer.next() {
        //     match (quote, tok, prev_token) {
        //         (Token::SingleQuote, Token::SingleQuote, Token::Backslash)
        //         |(Token::DoubleQuote, Token::DoubleQuote, Token::Backslash)
        //         | _
        //         => continue,

        //         (Token::SingleQuote, Token::SingleQuote, _)
        //         |(Token::DoubleQuote, Token::DoubleQuote, _)
        //         => {
        //             end = lexer.span().end;
        //             break;
        //         }
        //     }
        //     prev_token = token
        // }

        if let None = lexer.peek() {
            return Err(LexerError::Unexpected {
                got: Token::EOF, expected: quote
            });
        }

        Ok((Token::String, slice: start..end))
    }
}

impl Lexicon {
    pub fn is_whitespace(&self) -> bool { unimplemented!() }
    pub fn is_unit(&self) -> bool { unimplemented!() }
    pub fn is_operator(&self) -> bool { unimplemented!() }
    pub fn is_keyword(&self) -> bool { unimplemented!() }
    pub fn is_asn(&self) -> bool { unimplemented!() }
}

pub fn tokenize(src: &str) -> Result<Vec<ParsedToken>, LexerError>
{
    let mut collection = Vec::with_capacity(src.len());
    let mut lexer = Token::lexer(src).peekable();

    while let Some((token, i)) = lexer.next().enumerate()
    {
        let parsed = match token {
            Token::DoubleQuote | Token::SingleQuote =>
                Token::string(&mut lexer, token, seq)?,

            Token::Char | Token::Underscore =>
                Token::identifier(&mut lexer, seq)?,

            Token::Digit =>
                Token::integer(&mut lexer, seq)?,

            Token::Error =>
                return Err(Lexer::Error::BadToken(lexer.span())),

            x => ParsedToken {
                token: x,
                slice: lexer.span(),
                seq
            }
        };
        collection.push(parsed);
    }

    collection.push(ParsedToken {
        token: Token::EOF,
        seq: collection.last().unwrap().seq + 1,
        slice: {
            let span = lexer.span();
            (span.start+1)..(span.start+1)
        },
    });

    return collection;
}


// mod tests {
//     use super::*;

//     #[test]
//     fn string()
//     {
//         let answer: &[Token] =
//         &[
//             Token::LesserThanEql, Token::GreaterThanEql,
//             Token::IsEql, Token::NotEql, Token::AddEql,
//             Token::SubEql, Token::And, Token::Or,
//             Token::ShiftRight,

//             Token::BitOrEql, Token::BitAndEql, Token::BitNotEql,
//             Token::LesserThanEql, Token::Eql, Token::GreaterThanEql,
//             Token::Eql, Token::IsEql, Token::LesserThan,
//             Token::IsEql,


//             Token::GreaterThan, Token::GreaterThanEql,
//             Token::Eql, Token::LesserThan, Token::Eql,
//             Token::IsEql,

//             Token::Eql,
//             Token::IsNotEql,
//             Token::Eql,
//             Token::IsEql,
//             Token::Not,
//             Token::Add,
//             Token::AddEql,
//             Token::Sub,
//             Token::SubEql,
//             Token::Eql,
//             Token::Add,
//             Token::Add,
//             Token::Eql,

//             Token::Sub,
//             Token::Sub,
//             Token::AddEql,

//             Token::Eql,
//             Token::SubEql,
//             Token::Eql,
//             Token::IsEql,
//             Token::Add,
//             Token::IsEql,
//             Token::Sub,

//             Token::And,
//             Token::Amper,

//             Token::Or,
//             Token::Pipe,

//             Token::ShiftRight,
//             Token::GreaterThan,
//             Token::ShiftRight,
//             Token::Eql,
//             Token::ShiftLeft,
//             Token::Eql,
//             Token::ShiftLeft,
//             Token::LesserThan,
//             Token::BitOrEql,
//             Token::Eql
//             // --

//             //Token::Or,
//             //Token::GreaterThan,
//         ];

//         let src = r#"<= >= == != += -= && || >>
//             << |= &= ~= <== >== ==< ==>
//             >== <== === !== ==! ++= --=
//             =++ =-- +== -== ==+ ==- &&&
//             ||| >>> >>= <<= <<< |=="#;
//             // "==| ~~= ~==",


//         let mut lexer = Token::lexer(src).peekable();

//         for (i, tok) in lexer.enumerate()
//         { assert_eq!(tok, answer[i]) }
//     }
// }


fn main() {
    println!("Hello, world!");
}
