mod errors;
mod parse;
mod semantic;
mod tokens;
mod flycheck;
use std::io::stdin;

use itertools::Itertools;
use logos::Logos;
use tokens::TokenType;


fn main() {

    let mut buf = String::new();

    while let Ok(nread) = stdin().read_line(&mut buf) {
        buf.push('\n');
    }

    TokenType::lexer(&buf);




}
