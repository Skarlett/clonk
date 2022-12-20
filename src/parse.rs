use crate::errors::LexerError;
use crate::tokens::{Token, TokenType, ParseProduct};
use logos::{Lexer, SpannedIter};

#[derive(Debug, Eq, PartialEq, Clone, Copy)]
enum OpAssoc {
    Left,
    Right,
    NonAssoc,
}

#[derive(Debug, Clone)]
enum Item {
    Regular(Token),
    Product(ParseProduct),
}

impl Item {
    // fn ttype(&self) -> Token {}
}

#[derive(Debug, Clone, Default)]
struct ParserState {
    op_stack: Vec<Item>,
    queue: Vec<Item>,
 //   grouping: Vec<Item>,
}

impl ParserState {
    /// Flush items in the operator stack
    /// until an opening brace type is found
    fn flush_ops(&mut self) {
        const brace: [TokenType; 3] = [
            TokenType::BracketOpen,
            TokenType::BraceOpen,
            TokenType::ParamOpen,
        ];

        // Pop items from operator stack
        self.queue.extend(
            self.op_stack
                .drain(..)
                .rev()
                .take_while(|x|
                    if let Item::Regular(x) = x {
                        !brace.contains(&x.ttype)
                    } else {
                        true
                    }
                )
            );

    }
}

/// order of operations
impl TokenType {
    fn precendence(&self) -> u8 {
        match self
        {
            Self::Dot => 14,
            Self::Not => 13,
            Self::Pow => 12,

            Self::Div => 11,
            Self::Mul => 11,
            Self::Mod => 11,

            Self::Add => 10,
            Self::Sub => 10,

            Self::ShiftLeft => 9,
            Self::ShiftRight => 9,

            Self::GreaterThan => 8,
            Self::LesserThan => 8,
            Self::LesserThanEql => 8,
            Self::GreaterThan => 8,

            Self::IsNotEql => 7,
            Self::IsEql => 7,

            Self::Amper => 6,
            Self::Pipe => 5,
            Self::And => 4,
            Self::Or => 3,

            Self::BitAndEql => 2,
            Self::BitNotEql => 2,
            Self::BitOrEql => 2,

            Self::AddEql => 2,
            Self::SubEql => 2,
            Self::Eql => 2,

            _ => panic!("only operators should see their precendence"),
        }
    }

    fn is_keyword_dual_operator(&self) -> Option<[ParseProduct; 2]> {
        let prod = match self {
            TokenType::For => [ParseProduct::ForBody, ParseProduct::ForArgs],
            TokenType::While => [ParseProduct::WhileBody, ParseProduct::WhileCond],
            TokenType::If => [ParseProduct::IfBody, ParseProduct::IfCond],
            TokenType::Fn => [ParseProduct::FnBody, ParseProduct::FnSig],
            _ => return None
        };
        Some(prod)
    }

    /// operator assciotation
    fn assoc(&self) -> OpAssoc {
        match self {
            Self::ParamOpen
            | Self::ParamClose
            | Self::BraceClose
            | Self::BraceOpen
            | Self::BracketClose
            | Self::BracketOpen => {
                #[cfg(debug_assertions)]
                panic!("association called on non-associative");

                OpAssoc::NonAssoc
            },

            Self::Pow | Self::Not | Self::Tilde => OpAssoc::Right,

            _ => OpAssoc::Left,
        }
    }
}

fn parser<'a>(
    tokens: Lexer<TokenType>,
    postfix: &mut Vec<Token>,
) -> Result<Vec<Item>, LexerError<'a>> {
    let mut state = ParserState::default();
    let mut tokens = tokens.spanned().peekable();

    while let Some((ttype, span)) = tokens.next()
    {
        if ttype.is_unit()
        {
            state.queue.push(Item::Regular(Token::new(ttype, span)));
        }
        else if ttype.is_delimiter() || ttype.is_close_brace()
        {
            state.flush_ops();
        }
        // placed when group ends
        else if let Some(ops) = ttype.is_keyword_dual_operator() {
            state.op_stack.extend(
                ops.map(|item| Item::Product(item))
            );
        } else if ttype.is_open_brace() {

            state.op_stack.push(Item::Regular(Token::new(ttype, span)));

        } else if ttype.is_close_brace() {
            let inverted = ttype.invert_brace();
            state.flush_ops();

            if let Some((tok, _span)) = tokens.peek() {
                match tok {
                    TokenType::ParamOpen => {}
                    TokenType::BracketOpen => {}
                    TokenType::BraceOpen => {}
                    _ => {}
                }
            }
        } else if ttype == TokenType::Return {
        } else if ttype == TokenType::Import {
        } else if ttype.is_bin_operator() || ttype.is_assign_operator() {

            let o1 = ttype;
            let po1 = o1.precendence();

            while let Some(o2) = state.op_stack.pop() {
                let po2 = match &o2 {
                    Item::Regular(tok) => tok.ttype.precendence(),
                    _ => 0,
                };

                if po2 > po1 || (po2 == po1 && o1.assoc() == OpAssoc::Left) {
                    state.queue.push(o2);
                } else {
                    state.op_stack.push(o2);
                    break;
                }
            }

            state.op_stack.push(Item::Regular(Token::new(o1, span)));
        } else {

            eprintln!("token {:?} fell out of loop", ttype);
        }
    }

    state.queue.extend(state.op_stack.drain(..).rev());
    return Ok(state.queue);
}
