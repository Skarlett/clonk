#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../lexer/lexer.h"
#include "../lexer/debug.h"

#include "expr.h"


const char * print_datatype(enum DataType t) {
    switch (t) {
        case DT_StringT: return "string";
        case DT_IntT: return "integer";
        case DT_NullT: return "null";
        default: return "undefined";
    }
}

const char * print_operator_name(enum Operation t) {
    switch (t) {
        case Add: return "addition";
        case Sub: return "subtract";
        case Multiply: return "multiply";
        case Divide: return "divide";
        case Modolus: return "modolus";
        case Pow: return "power";
        case And: return "and";
        case Or: return "or";
        case GtEq: return "gteq";
        case Gt: return "gt";
        case Lt: return "lt";
        case LtEq: return "lteq";
        case IsEq: return "iseq";
        case NotEq: return "noteq";
        default: return "unknown";
    }
}

const char * print_operator_symbol(enum Operation t) {
    switch (t) {
        case Add: return "+";
        case Sub: return "-";
        case Multiply: return "*";
        case Divide: return "/";
        case Modolus: return "%";
        case Pow: return "^";
        case And: return "&&";
        case Or: return "||";
        case GtEq: return "<=";
        case Gt: return "<";
        case Lt: return ">";
        case LtEq: return ">=";
        case IsEq: return "==";
        case NotEq: return "!=";
        case Access: return ".";
        default: return "unknown";
    }
}


void draw_token_error_at(const char * line, struct Token *token) {
    printf("'%s'\n", line);
    printf("-");
    for (int i = 0; token->start > i; i++) {
        printf("-");
    }
    
    printf("^\n");
}
