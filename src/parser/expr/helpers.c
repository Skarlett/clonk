#include <stdint.h>
#include <string.h>
#include "expr.h"
#include "../lexer/lexer.h"
#include "../../prelude.h"

// int cmpexpr(struct Expr *a, struct  Expr *b) {
//     // pattern matching would be awesome right now.

//     if (a->type != b->type) {
//         return 0;
//     }

//     else if (a->type == UndefinedExprT) {
//         return 1;
//     }
    
//     else if (a->type == BinExprT) {
//         return cmpexpr(a->inner.bin.lhs, b->inner.bin.lhs) == 1
//             && cmpexpr(a->inner.bin.rhs, b->inner.bin.rhs) == 1;
//     }
    
//     else if (a->type == UniExprT) {
//         if (a->inner.uni.op != b->inner.uni.op) { return 0; }

//         if (a->inner.uni.op == UniOpNop) { return 1; }

//         if (a->inner.uni.op == UniCall) {
//             if (a->inner.uni.interal_data.fncall.name_length == b->inner.uni.interal_data.fncall.name_length
//                 && a->inner.uni.interal_data.fncall.args_length == b->inner.uni.interal_data.fncall.args_length) {
//                 int ret;
                
//                 if (strncmp(
//                     a->inner.uni.interal_data.fncall.func_name,
//                     b->inner.uni.interal_data.fncall.func_name,
//                     a->inner.uni.interal_data.fncall.name_length) == 0)
//                 {
//                     for (usize i=0; a->inner.uni.interal_data.fncall.args_length > i; i++) {
//                        /*TODO*/
//                     }
//                     return 1;
//                 }
//             }
//             else {return 0;}
//         }

//         // Compare Value and Variables
//         else if (a->inner.uni.op == UniValue) {
            
//             // Compare symbol tags, and then determine if the expression is a variable or a value
//             if (a->inner.uni.interal_data.symbol.tag == b->inner.uni.interal_data.symbol.tag)
//             {
//                 // This is a Value
//                 if (a->inner.uni.interal_data.symbol.tag == ValueTag) {
                    
//                     // Check Values have same type
//                     if (a->inner.uni.interal_data.symbol.inner.value.type !=
//                         b->inner.uni.interal_data.symbol.inner.value.type) { return 0; }

//                     // Compare literal values.
//                     switch (a->inner.uni.interal_data.symbol.inner.value.type) {
//                         case IntT:
//                             return a->inner.uni.interal_data.symbol.inner.value.data.integer == b->inner.uni.interal_data.symbol.inner.value.data.integer; 
                        
//                         case StringT:
//                             if (a->inner.uni.interal_data.symbol.inner.value.data.string.length != b->inner.uni.interal_data.symbol.inner.value.data.string.length) {
//                                 return 0;
//                             }
//                             return strncmp(
//                                 a->inner.uni.interal_data.symbol.inner.value.data.string.ptr,
//                                 b->inner.uni.interal_data.symbol.inner.value.data.string.ptr,
//                                 a->inner.uni.interal_data.symbol.inner.value.data.string.length
//                             ) == 0;
                        
//                         // Null Datatype
//                         case NullT:
//                             return 1;

//                         // Should be unreachable   
//                         default:
//                             return -1;
//                     }
//                 }

//                 // This is a variable
//                 else if (a->inner.uni.interal_data.symbol.tag == VariableTag) {
//                     return strcmp(
//                         a->inner.uni.interal_data.symbol.inner.variable,
//                         b->inner.uni.interal_data.symbol.inner.variable
//                     ) == 0;
//                 }

//                 // NullTag 
//                 else { return 1; }
//             }
//             // a...symbol.tag == b...symbol.tag
//             else { return 0; }
//         }
//     }
// }

// size_t _expr_len_inner(Expr *expr, size_t current) {
//     if (expr->type==BinExprT) {
//         current = _expr_len_inner(expr->inner.bin.rhs, current+1);
//     }
//     return current;
// }

// size_t get_expr_len(Expr *expr) {
//     return _expr_len_inner(expr, 1);
// }

inline int is_expr(
    char *line,
    Token tokens[],
    usize ntokens)
{   
    return (
        tokens[0].token == NOT
        || tokens[0].token == PARAM_OPEN
        || is_data(tokens[0].token)
        //|| is_func_call(tokens, ntokens)
    );
}

inline int is_expr_token(enum Lexicon token) {
    return token == NOT || 
        token == PARAM_OPEN || 
        token == PARAM_CLOSE || 
        is_data(token) ||
        is_bin_operator(token);
}

// int assignment_from_token(enum Lexicon t){
//     switch (t) {
//         case MINUSEQ: return EqSub;
//         case PLUSEQ: return EqAdd;
//         case EQUAL: return Eq;

//         default: return BinOpNop;
//     }
// }

inline int binop_from_token(enum Lexicon t){
    switch (t) {
        case ADD: return Add;
        case SUB: return Sub;
        case MUL: return Multiply;
        case DIV: return Divide;
        case MOD: return Modolus;
        case POW: return Pow;
        case AND: return And;
        case OR: return Or;
        case ISEQL: return IsEq;
        case ISNEQL: return NotEq;
        case GTEQ: return GtEq;
        case LTEQ: return LtEq;
        case LT: return Lt;
        case GT: return Gt;
        default: return BinOpNop;
    }
}


/* ------------------------------------------ */
/*            symbols & values                */
/* ------------------------------------------ */

int symbol_from_token(const char *line, struct Token token, struct Symbol *value) {
//     if (token.token == WORD) {
//         // Variable *var = xmalloc(sizeof(Variable));
//         value->tag = VariableTag;
//         value->inner.variable = xmalloc(token.end - token.start);
//         strncpy(value->inner.variable, line + token.start, token.end - token.start);
//     }

//     else if (token.token == INTEGER || token.token == STRING_LITERAL) {
//         //Value *val = xmalloc(sizeof(Value));
//         value->tag = ValueTag;
//         //value->data_ptr=val;
        
//         if (token.token == INTEGER) {
//             value->inner.value.type = IntT;
//             value->inner.value.data.integer = strtol((line + token.start), NULL, 10);
//         }
//         else {
//             value->inner.value.type = StringT;
//             value->inner.value.data.string.length = token.end - token.start;
//             value->inner.value.data.string.capacity = value->inner.value.data.string.length + 1; 

//             value->inner.value.data.string.ptr = calloc(value->inner.value.data.string.capacity, sizeof(char));
            
//             strncpy(
//                 value->inner.value.data.string.ptr,
//                 line + token.start,
//                 value->inner.value.data.string.length
//             );
//         }
//     }

//     else 
//         return -1;
    
//     return 0;
}
