// CLONK interpreter
// my very own 1990s retro built interpreter
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define TRUE 1
#define FALSE 0
#define EXTENSION "ka"

#define ALPHABET "asdfghjkklqwertyuiopzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM"
#define DIGITS "1234567890"

#define STR_STACK_SIZE 64
#define FUNC_ARG_SIZE 8
#define STMT_CAPACITY 16

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

    // conditional block 
    Condition,

    Block,

    Return
};

// variable OR literal
struct BaseValue {
    void *data_ptr;
    enum Tag tag;
    enum DataType datatype;
};

void init_basevalue(struct BaseValue *v) {
    v->tag=Literal;
    v->datatype=Null;
    v->data_ptr=0;
}

int basevalue_from_token(char *line, struct Token token, struct BaseValue *value) {
    init_basevalue(value);
    
    if (token.token == WORD) {
        value->tag = Variable;
    }

    else if (token.token == INTEGER) {
        value->tag = Literal;
        value->datatype = Int;
        value->data_ptr = strtol((line + token.start), NULL, 10);
    }

    else if (token.token == STRING_LITERAL) {
        value->tag = Literal;
        value->datatype = MallocString;

        char *inner_data = calloc(STR_STACK_SIZE, sizeof(char));
        for (int i; token.end > i; i++) {
            inner_data[i] = (line + token.start)[i];
        }
        
        value->data_ptr = inner_data;
    }
    else 
        return -1;
    
    return 0;
}

struct Statement {
    enum StatementType type;
    void *internal_data;
};

struct BlockStatement {
    struct Statement *statements;
    unsigned long capacity;
    unsigned long length;
};

inline void init_block(struct BlockStatement *block, unsigned long capacity) {
    block->statements = malloc(sizeof(struct Statement)*capacity);
    block->capacity=capacity;
    block->length=0;
}

inline void append_statement(struct BlockStatement *block, struct Statement stmt) {
    if (block->length >= block->capacity) {
        block->capacity *= 2;
        block->statements=realloc(block->statements, block->capacity);
    }

    block->length += 1;
    block->statements[block->length] = stmt;
}

inline int is_block(struct Token tokens[], int ntokens) {
   return (tokens[ntokens].token == CLOSE_BRACK) && (tokens[0].token == OPEN_BRACK);
}

struct ReturnStatement {
    struct BaseValue value;
};

inline int is_return_statement(char *line, struct Token tokens[], int nstmt) {
    for (int i=0; 6 > i; i++)
        if ((tokens[0].start + line)[i] != "return"[i])
            return FALSE;
    
    return TRUE;
}

int construct_ret_statement(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
    struct BaseValue var;
    struct ReturnStatement *ret_stmt = malloc(sizeof(struct ReturnStatement));
    basevalue_from_token(line, tokens[1], &var);
    stmt->internal_data=ret_stmt;
    stmt->type=Return;

}

struct DeclareStatement {
    int name_sz;
    char name[STR_STACK_SIZE];
    struct BaseValue data;
};

// BinOp

struct ExprStatement {
    struct BaseValue base;
    enum BinOperation op;
    struct ExprStatement *other;
};

void init_expr_stmt(struct ExprStatement *expr) {
    expr->op=Nop;
    expr->other=0;
    init_basevalue(&expr->base);
}

// 2 + 2

inline int is_express_statement(struct Token tokens[], int nstmt) {
    if (nstmt % 2 == 0)
        return -1; 
    
    for (int i=0; nstmt > i; i++) {        
        if (tokens[1].token != SUB || tokens[1].token != ADD)
            return -1;
    }

    return 0;
}

int inner_expr_stmt(char *line, struct Token tokens[], int nstmt, struct ExprStatement *ex_stmt){
    struct ExprStatement *expr = malloc(sizeof(struct ExprStatement));
    init_expr_stmt(expr);
    //free(expr->base.data_ptr);
    basevalue_from_token(line, tokens[0], &expr->base);
    
    if (nstmt > 1) {
        if (tokens[1].token == ADD)
            expr->op=Add;
        else if (tokens[1].token == SUB)
            expr->op=Sub;
        else
            return -1;
    
        struct ExprStatement *o_expr = malloc(sizeof(struct ExprStatement));
        inner_expr_stmt(line, tokens+2, nstmt-2, o_expr);
        expr->other = o_expr;
        return 0;
    }

    return (nstmt == 0)*-1;
}

int construct_expr_stmt(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
    struct ExprStatement *expr = malloc(sizeof(struct ExprStatement));
    init_expr_stmt(expr);

    inner_expr_stmt(line, tokens, nstmt, expr);

    stmt->internal_data=expr;
    stmt->type=Expression;
    return 0;
}

struct FunctionDefinition {
    int name_sz;
    int param_sz;

    char func_name[STR_STACK_SIZE];
    char **parameters[FUNC_ARG_SIZE];
};

void init_func_def(struct FunctionDefinition *fn) {
    struct BlockStatement body;
    init_block(&body, STMT_CAPACITY);

    fn->name_sz = 0;
    fn->param_sz = 0;

    for (int i=0; FUNC_ARG_SIZE > i; i++){
        fn->parameters[i] = 0;
    }
}

// word('def') word open_param ... close_param
inline int is_func_definition(char *line, struct Token tokens[], int nstmt) {
    if (tokens[0].token == WORD) {
        for (int i=0; 3 > i; i++)
            if ((line + tokens[0].start)[i] != "def"[i]) 
                return 0;
        

        return tokens[1].token == WORD  
                && tokens[2].token == PARAM_OPEN
                && tokens[nstmt].token == PARAM_CLOSE;
    } else
        return 0;
}

int construct_func_definition(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
    struct FunctionDefinition *proceedure = malloc(sizeof(struct FunctionDefinition));
    init_func_def(proceedure);
    strncpy(proceedure->func_name, (line + tokens[1].start), tokens[1].end);
    

    for (int i=2; nstmt-1 > i; i++) {
        if (tokens[i].token == COMMA) continue;
        char *parameter = malloc(tokens[i].end - tokens[i].start);
        strncpy(parameter, line + tokens[i].start, tokens[i].end);

        if (proceedure->param_sz > FUNC_ARG_SIZE)
            return -1;
        
        *proceedure->parameters[proceedure->param_sz] = parameter;
        proceedure->param_sz += 1;
    }

    stmt->internal_data = proceedure;
    stmt->type = Define;

    return 0;
}

struct FunctionCall {
    int name_sz;
    int args_sz;

    char *func_name;
    struct BaseValue args[FUNC_ARG_SIZE];
};

void init_func_call(struct FunctionCall *fn) {
    fn->name_sz = 0;
    fn->args_sz = 0;
    for (int i=0; FUNC_ARG_SIZE > i; i++){
        init_basevalue(&fn->args[i]);
    }
}

// word open param ... close param
inline int is_func_call(struct Token tokens[], int nstmt) {
    return tokens[0].token == WORD
    && tokens[1].token == PARAM_OPEN
    && tokens[nstmt].token == PARAM_CLOSE;
}

int construct_func_call(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
    struct FunctionCall *fn_stmt = malloc(sizeof(struct FunctionCall));
    fn_stmt->name_sz = 0;
    fn_stmt->args_sz = 0;

    for (int i=0; FUNC_ARG_SIZE > i; i++){
        init_basevalue(&fn_stmt->args[i]);
    }
    
    // setup name
    fn_stmt->name_sz=tokens[0].end - tokens[0].start;
    char *fname = malloc(fn_stmt->name_sz);
    strncpy(fname, tokens[0].start + line, tokens[0].end);
    fn_stmt->func_name = fname;

    for (int i=1; nstmt-1 > i; i++) {
        if (tokens[i].token == COMMA) continue;

        struct BaseValue base = {
            .datatype=Null,
            .data_ptr=0,
            .tag=Literal
        };
        
        if (basevalue_from_token(line, tokens[i], &base) != 0) {
            fprintf(stderr, "error in basevalue_from_token");
            exit(1);
        }
        
        if (fn_stmt->args_sz > FUNC_ARG_SIZE)
            return -1;
        
        fn_stmt->args[fn_stmt->args_sz] = base;
        fn_stmt->args_sz += 1;
    }
    
    stmt->internal_data=fn_stmt;
    stmt->type=CallFunc;

    return 0;
}

enum ConditionState {
    If,
    Elif,
    Else
};

struct ConditionStatement {
    struct ExprStatement expr;
    enum ConditionState state;
//    struct BlockStatement if_true;
//    struct BlockStatement if_false;
};

int init_condition_stmt(struct ConditionStatement *stmt) {
    struct BlockStatement block;
    stmt->state=If;
    init_expr_stmt(&stmt->expr);
}

// word('if') open param expr close param
inline int is_conditional_definition(char *line, struct Token tokens[], int nstmt) {
    char keyword[4];
    memset(keyword, 0, 4);

    for (int i=0; 4 > i; i++) {
        keyword[i] = (line + tokens[0].start)[i];
    }
    
    return tokens[0].token == WORD
    && (strcmp(keyword, "if")
        || strcmp(keyword, "elif")
        || strcmp(keyword, "else"))
    && tokens[1].token == PARAM_OPEN
    && tokens[nstmt].token == PARAM_CLOSE;
}


int construct_condition_statement(char *line, struct Token tokens[], int nstmt, struct Statement *stmt) {
    struct ConditionStatement *condition_stmt = malloc(sizeof(struct ConditionStatement));
    init_condition_stmt(condition_stmt);

    struct Statement *expr_stmt = malloc(sizeof(struct Statement));
    construct_expr_stmt(line, tokens+2, nstmt-1, expr_stmt);

    stmt->internal_data = condition_stmt;
    stmt->type = Condition;

    return 0;
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

int construct_declare_statement(char *line, struct Token tokens[], struct Statement *stmt) {
    struct DeclareStatement *dec_stmt = malloc(sizeof(struct DeclareStatement));
    struct BaseValue data;
    int name_len = 0;
    dec_stmt->name_sz = 0;
    init_basevalue(&dec_stmt->data);
    
    for (int i=0; 3 > i; i++) {
        if (i == 1)
            continue;

        else if (i == 0 && tokens[i].token != WORD)
            return -1;

        else if (i==0) {
            char *slice = (line + tokens[i].start);

            int c=0;
            for(;tokens[i].end > c ;c++)
                dec_stmt->name[c] = slice[c];
            
            dec_stmt->name_sz=c;
        }

        // copy identifier
        else if (i == 2) {
            if (basevalue_from_token(line, tokens[i], &data) != 0) {
                fprintf(stderr, "error in basevalue_from_token");
                exit(1);
            }
            dec_stmt->data=data;
        }
    }
    // poo, more heap allocations
    stmt->internal_data = dec_stmt;
    stmt->type = Declare;
    return 0;
}


int construct_statement(char *line, struct Token tokens[], long unsigned nstmt, struct BlockStatement *block) {
    // 2 + 2
    if (is_express_statement(tokens, nstmt))
        return construct_expr_stmt(line, tokens, nstmt, block->statements);
    
    // declare var
    else if (is_declare_statement(tokens, nstmt))
        return construct_declare_statement(line, tokens, block->statements);
    
    // foo( ... )
    else if (is_func_call(tokens, nstmt)) 
        return construct_func_call(line, tokens, nstmt, block->statements);
    
    // def foo((T),*)
    else if (is_func_definition(line, tokens, nstmt)) {
        //*expects_next = Block;
        return construct_func_definition(line, tokens, nstmt, block->statements);
    }

    // if ( expr )
    else if (is_conditional_definition(line, tokens, nstmt)) {
        //*expects_next = Block;
        return construct_condition_statement(line, tokens, nstmt, block->statements);
    }
    
    else if (is_block(tokens, nstmt)) {
        struct BlockStatement new_block;
        init_block(&new_block, STMT_CAPACITY);
        construct_statement(line, tokens + 1, nstmt, &new_block);
            struct Statement new = {
            .type = Block,
            .internal_data = &new_block
        };
        append_statement(block, new);
        return 0;
    }

    else if (is_return_statement(line, tokens, nstmt))
        return construct_ret_statement(line, tokens, nstmt, block->statements);
    
    return -1;
}

int create_ast(char *line, struct Token tokens[], long unsigned ntokens) {

    struct BlockStatement root;
    init_block(&root, STMT_CAPACITY*2);
    
    unsigned long last_stmt = 0;
    for(long unsigned i=0; ntokens > i; i++) {
        enum Lexicon token = tokens[i].token;

        if (token != SEMICOLON 
            || token != PARAM_CLOSE
            || token != CLOSE_BRACK)
        {
            if (construct_statement(line, tokens + last_stmt, i, &root) != 0) {
                break;
            }
            last_stmt += i;
        }
    }
    
    return 0;
}


int parse(char *filepath) {
    int buf_sz = 2048;
    char line[buf_sz];
    long unsigned token_n = 0;
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
        //create_abstree(line, tokens, ntokens, buf_sz);

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
