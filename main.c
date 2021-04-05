// CLONK interpreter

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0
#define EXTENSION "ka"

#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"

#define STR_STACK_SIZE 128

enum Error {
    OK = 0,
    ExpectToken,
};

enum Lexicon {
    NULLTOKEN,
    
    // Ignore token (whitespace, newline, carriage return)
    WHITESPACE,

    // {
    OPEN_BRACK,
    
    // }
    CLOSE_BRACK,
    
    // (
    PARAM_OPEN,
    
    // )
    PARAM_CLOSE,

    // +
    ADD,

    // -
    SUB,

    // =
    EQUAL,

    // "
    QUOTE,
    
    // ;
    SEMICOLON,

    // a-zA-Z
    CHAR,

    // 0-9
    DIGIT,
  
    // ~!@#$%^&*_+=-`
    SPECIAL_CHAR,

    // ,
    COMMA,
    //********* START OF COMPLEX TOKENS ********
    // Complex tokens wont show up in the first round of lexer'ing
    // they're generated from combinations of tokens
    // "fn"
    //********* START OF COMPLEX LEXICONS ********

    // [NUM, ..] WHITESPACE|SEMICOLON   
    // 20392
    INTEGER,

    // [CHARACTER, ..] WHITESAPCE|SEMICOLON
    // something
    WORD,

    // [QUOTE, ... QUOTE]
    // something
    STRING_LITERAL,

    UNKNOWN,
};


const char * ptoken(enum Lexicon t) {
    if (t == INTEGER) return "integer";
    else if (t == WORD) return "word";
    else if (t == NULLTOKEN) return "nulltoken";
    else if (t == WHITESPACE) return "whitespace";
    else if (t == ADD) return "add";
    else if (t == OPEN_BRACK) return "brack_open";
    else if (t == CLOSE_BRACK) return "brack_close";
    else if (t == PARAM_OPEN) return "param_open";
    else if (t == PARAM_CLOSE) return "param_close";
    else if (t == COMMA) return "comma";
    else if (t == DIGIT) return "digit";
    else if (t == QUOTE) return "quote";
    else if (t == EQUAL) return "eq";
    else if (t == SUB) return "sub";
    else if (t == SEMICOLON) return "semi-colon";
    else if (t == UNKNOWN) return "unknown";
    else if (t == SPECIAL_CHAR) return "special_char";
    else if (t == CHAR) return "char";
     else if (t == STRING_LITERAL) return "str_literal";
    else return "PTOKEN_ERROR_UNKNOWN_TOKEN";
}

enum Lexicon tokenize_char(char c) {
    if (' ' == c
        || c == '\n'
        || c == '\r'
        || c == '\t' )
        return WHITESPACE;
    else if (c == '+')  return ADD;
    else if (c == '-')  return SUB;
    else if (c == '=')  return EQUAL;
    else if (c == '"')  return QUOTE;
    else if (c == '{')  return OPEN_BRACK;
    else if (c == '}')  return CLOSE_BRACK;
    else if (c == '(')  return PARAM_OPEN;
    else if (c == ')')  return PARAM_CLOSE;
    else if (c == ';')  return SEMICOLON;
    else if (c == ',')  return COMMA;

    for (int i = 0; sizeof(DIGITS) > i; i++) {
        if (c == DIGITS[i])
            return DIGIT;
    }

    for (int i = 0; sizeof(ALPHABET) > i; i++) {
        if (c == ALPHABET[i])
            return CHAR;
    }

    return UNKNOWN;
}

struct Token {
    int start;
    int end;
    enum Lexicon token;
};

int tokenize(char *line,  struct Token tokens[], int token_idx) {
    enum Lexicon complex_token = NULLTOKEN;
    int complex_start = 0;
    int ctr = 0;
    int original = token_idx;

    enum Lexicon lexed;

    for (int i=0; strlen(line) > i; i++) {
        if (line[i] == 0) continue;
        lexed = tokenize_char(line[i]);
                
        if (complex_token == NULLTOKEN) {
            if (lexed == DIGIT || lexed == CHAR || lexed == QUOTE) 
            {
                if (lexed == DIGIT) 
                    complex_token = INTEGER;
                else if (lexed == CHAR)
                    complex_token = WORD;
                else // QUOTE
                    complex_token = STRING_LITERAL;
                
                complex_start = i;
                continue;
            }
        }
        else if (complex_token == INTEGER && lexed == DIGIT 
                || complex_token == WORD && lexed == CHAR
                || complex_token == STRING_LITERAL && lexed != QUOTE )
            continue;
        
        else if (complex_token == INTEGER || complex_token == WORD || complex_token == STRING_LITERAL) {
            struct Token token = {
                    .start = complex_start,
                    .end = i,
                    .token = complex_token
            };

            complex_start = 0;
            tokens[token_idx+ctr] = token;
            ctr += 1;
            // skip the extra quote token
            if (complex_token == STRING_LITERAL) {
                complex_token = NULLTOKEN;
                continue;
            }
            complex_token = NULLTOKEN;
        }

        if (lexed != WHITESPACE) {
            struct Token token = {
                .start = i,
                .end = i,
                .token = lexed
            };

            complex_token = NULLTOKEN;
            complex_start = 0;
            tokens[token_idx+ctr] = token;
            ctr += 1;
        }
    }
    
    if (complex_token != NULLTOKEN) {
        struct Token token = {
            .start = complex_start,
            .end = strlen(line),
            .token = complex_token
        };
        tokens[token_idx+ctr] = token;
        ctr += 1;
    }

    return ctr;
}

// block([statements..]*)
// statement (x = "something")
// expressions (1+1)

int get_size(enum Lexicon type) {
    if (INTEGER == type)
        return 8;
    
    else if (STRING_LITERAL == type)
        return 8;
    
    else
        return -1;
}

struct Value {
    enum Lexicon token;
    void *ptr;
};

struct Statement {
    void *function;
    struct Value args[8];
    int idx_args;
};

struct Statements {
    // ([statement, ..], statement)
    struct Statements *statements;
    struct Statement *statement;
};

int declare_variable(void *ret, struct Value word, struct Value value) {
    return -1;
}

// struct Block {
//     // [ ([statement, ..], statement), ..]
//     struct Statements *statements;
// };

// struct look_up_map
// * = required
// [
//   expression - expresses arithmitic
//   [int* ADD|SUB expression|int]
//   
//   declare - allocate memory and write to it
//   [word* eq* int|str_literal*]
//  
//   block - list of nested statements
//   [open_brace*, .. close_brace&]
//   
//   if - conditional statement (first block is if condition is true, second is false)
//   [word('if')*, expression*, *block, *block]
//   
//   define - creates a function
//   [word('fn')*, open_param* .. closed_param*, block*]
// ]
//
//

enum BinOperation {
    // no operation
    Nop,
    // concat
    Add,
    // subtract
    Sub
};

enum Tag {
    Variable,
    Literal
};

enum DataType {
    Null,
    Int,
    MallocString,
};

enum StatementType {
    Undefined,

    // define function (named block)
    Define,
    
    // declare var
    Declare,

    // call a block 
    CallFunc,

    //Math Expression (with variables)
    Expression,

    // read out variables value
    //Evaluate,

    // conditional block 
    If
};


struct Block {
    struct Statement *statements;
    int capacity;
    int length;
};

// variable OR literal
struct BaseValue {
    void *data_ptr;
    enum Tag tag;
    enum DataType datatype;
};

struct Statement {
    enum StatementType type;
    void *internal_data;
};

struct DeclareStatement {
    char name[STR_STACK_SIZE];
    struct BaseValue data;
};

struct DefineStatement {
    char name[STR_STACK_SIZE];
    struct BaseValue *arguments[8];
    struct Block statements;
};

struct ExprStatement {
    struct BaseValue left;
    struct BaseValue right;
    enum BinOperation op;
};

struct FcallStatement {
    char func_name[STR_STACK_SIZE];
    void *args[8];
};

struct IfStatement {
    struct ExprStatement condition;
    struct Block if_true;
    struct Block if_false;
};


int production(struct Token tokens[], int n_written, struct Block *block, int block_sz) {
    int i = 0;
    int statements_n = 0;
    
    while (n_written > i){
        for(i; n_written > i; i++) {
            if (tokens[i].token != SEMICOLON) 
                statements_n += 1;
            else
                break;
        }
        
        for (int stmt_idx=0; statements_n > stmt_idx; stmt_idx++) {
            struct Statement stmt = {
                .type=Undefined,
                .internal_data=NULL
            };
            
            //figure_out_statement();

            //block->statements[stmt_idx] = ;
        }
    }
}


// 2 + 2
inline int is_express_statement(struct Token tokens[], int ntokens) {
    return ntokens == 3 && tokens[1].token == SUB || tokens[1].token == ADD;
}

// x = 200
inline int is_declare_statement(struct Token tokens[], int ntokens) {
    return ntokens == 3 
    && tokens[0].token == WORD
    && tokens[1].token == EQUAL
    && (
        tokens[2].token == STRING_LITERAL
        || tokens[2].token == INTEGER
        || tokens[2].token == WORD
    );
}

void construct_declare_statement(char *line, struct Token tokens[], struct Statement *stmt) {
    struct BaseValue base = {
        .datatype=Null,
        .data_ptr=0,
        .tag=Literal
    };

    char init_buf[STR_STACK_SIZE];
    memset(init_buf, 0, STR_STACK_SIZE);

    struct DeclareStatement dec_stmt = {
        .name=init_buf,
        .data=base
    };

    for (int i=0; 3 > i; i++) {
        char word_buf[STR_STACK_SIZE];
        memset(word_buf, 0, STR_STACK_SIZE);

        if (i == 1)
            continue;
        else if (i == 0 && tokens[i].token != WORD)
            return -1;

        char *slice = (line + tokens[i].start);
        
        for(int c=0; tokens[i].end ;c++)
            word_buf[c] = slice[c];
        
        // copy name
        if (i == 0)
            strcpy(word_buf, dec_stmt.name);
        
        // copy identif
        else if (i == 2) {
            int data_sz = (tokens[2].end - tokens[2].start);
            if (tokens[2].token == WORD || tokens[2].token == STRING_LITERAL) {
                char *inner_data = malloc(data_sz+1);
                memset(inner_data, 0, data_sz+1);
                strcpy(word_buf, inner_data);
                base.data_ptr = inner_data;
                base.tag = Variable;
            }
            else if(tokens[2].token == INTEGER) {
                base.data_ptr = strtol((line + tokens[2].start), NULL, 10);
                base.datatype = Int;
            }
            //strcpy(word_buf, dec_stmt.name);
        }

    }
    
}

void construct_expr_stmt(char *line, struct Token tokens[], struct Statement *stmt) {
    struct BaseValue default_integer = {
        .datatype=Int,
        .data_ptr=0,
        .tag=Literal
    };
    
    struct ExprStatement expr = {
        .left=default_integer,
        .right=default_integer,
        .op=Nop
    };

    for (int ctr=0; 3 > ctr ; ctr++) {
        struct BaseValue base = default_integer; 
        // skip the operation, we can figure that out easily
        if (ctr == 1) {
            if (tokens[ctr].token == ADD)
                expr.op = Add;
            else if (tokens[ctr].token == SUB)
                expr.op = Sub;
        }

        // Parse out Literal
        else if (tokens[ctr].token == INTEGER) {
            default_integer.data_ptr = strtol((line + tokens[ctr].start), NULL, 10);
        }

        // variable name used here
        else if (tokens[ctr].token == WORD) {
            int data_sz = (tokens[ctr].end - tokens[ctr].start);
            char *inner_data = malloc(data_sz+1);
            memset(inner_data, 0, data_sz+1);

            char *slice=(line + tokens[ctr].start);
            strncpy(slice, tokens[ctr].end, inner_data);
            base.tag = Variable;

        }

        if (ctr == 0)
            expr.left = base;
        else if (ctr == 2)
            expr.right = base;
    }
    //statement->internal_data =;
}


int construct_statement(char *line, struct Token tokens[], int ntokens, struct Statement *statement) {
    if (is_express_statement(tokens, ntokens))
        construct_expr_stmt(line, tokens, statement);
        
    else if (is_declare_statement(tokens, ntokens))
        construct_declare_statement(line, tokens, statement);

}

int parse(char *filepath) {
    int buf_sz = 2048;
    char line[buf_sz];
    int token_n = 0;
    struct Token tokens[buf_sz];
        
    FILE *fd;

    memset(line, 0, buf_sz);
    
    if ((fd = fopen(filepath, "r")) == NULL) {
        perror("Error! opening file");
        exit(1);
    }

    int n = 1;

    while (n > 0) {
        fgets(line, buf_sz, fd);

        //calculate the index/position of the last character written to the buffer
        for (long int i=0; buf_sz > i; i++) {
            if (line[i] == 0) {
                n=i;
                break;
            }
        }

        if (n <= 0) {
            break;
        }

        int ntokens = tokenize(line, tokens, token_n);
        printf("token stream: ");
        for (int i=0; ntokens > i; i++) {
            printf("[%s(%d,%d)] ", ptoken(tokens[i].token), tokens[i].start, tokens[i].end);
        }
        printf("\n");
        //printf("abstree stream:");
        create_abstree(line, tokens, ntokens, buf_sz);

        printf("\n");
        memset(line, 0, buf_sz);
    }
    fclose(fd);
    
    
    return 0;
}



int main(int argc, char *argv[]) {
    if (argc > 1) {
        parse(argv[1]);
        return 0;
    }
    else {
        printf("%s [file.%s]\n", argv[0], EXTENSION);
        return 1;
    }
}
