#include "ast.h"
#include "expr.h"

const char * print_datatype(enum DataType t) {
    switch (t) {
        case StringT: return "string";
        case IntT: return "integer";
        case NullT: return "null";
        default: return "undefined";
    }
}
const char * print_expr_t(enum ExprType t) {
    switch (t) {
        case UniExprT: return "unitary";
        case BinExprT: return "binary";
        case UndefinedExprT: return "null";
        default: return "undefined";
    }
}


const char * print_bin_operator(enum BinOp t) {
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
const char * p_bin_operator_sym(enum BinOp t) {
    switch (t) {
        case Add: return "+";
        case Sub: return "-";
        case Multiply: return "*";
        case Divide: return "/";
        case Modolus: return "\%";
        case Pow: return "^";
        case And: return "&&";
        case Or: return "||";
        case GtEq: return "<=";
        case Gt: return "<";
        case Lt: return ">";
        case LtEq: return ">=";
        case IsEq: return "==";
        case NotEq: return "!=";
        default: return "unknown";
    }
}

const char * print_symbol_type(enum Tag t) {
    switch (t) {
        case NullTag: return "NullTag";
        case VariableTag: return "Variable";
        case ValueTag: return "Literal";
        default: return "undefined";
    }
}

int print_expr(Expr *expr, short unsigned indent){
    tab_print(indent);
    printf("type: %s\n", print_expr_t(expr->type));
    tab_print(indent);
    printf("depth: %d\n", expr->depth);

    if (expr->type == UniExprT) {
        //struct UniExpr *uni = expr->inner_data;
        if (expr->inner.uni.op == UniValue) {
            tab_print(indent);
                
            printf("symbol type: %s\n", print_symbol_type(expr->inner.uni.interal_data.symbol.tag));
            tab_print(indent);
            printf("symbol val: ");
                
            switch (expr->inner.uni.interal_data.symbol.tag) {
                case VariableTag:
                    printf("symbol name: %s\n", expr->inner.uni.interal_data.symbol.inner.variable);
                    break;
                    
                case ValueTag:
                    switch (expr->inner.uni.interal_data.symbol.inner.value.type) {
                        case IntT:
                            printf("%d\n", expr->inner.uni.interal_data.symbol.inner.value.data.integer);
                            break;
                            
                        case StringT:
                            printf("%s\n", expr->inner.uni.interal_data.symbol.inner.value.data.string.ptr);
                            break;
                            
                        case NullT:
                            printf("null");
                            break;
                            
                        default: 
                            printf("error: got unexpected datatype");
                            break;
                        }

                        break;
                    
                    default:
                        printf("error: got unexpected tag");
                        break;
                }
        }
        

        else if (expr->inner.uni.op == UniCall) {
                // tab_print(indent);
                // printf("{\n");
                tab_print(indent);
                printf("function call: %s\n", expr->inner.uni.interal_data.fncall.func_name);
                tab_print(indent);
                printf("parameters: {\n");                
                for (int i=0; expr->inner.uni.interal_data.fncall.args_length > i; i++){
                    print_expr(expr->inner.uni.interal_data.fncall.args[i], indent+1);
                    if (expr->inner.uni.interal_data.fncall.args_length-1 != i) {
                        printf(",");
                    }
                }
                tab_print(indent);
                printf("}\n");
                //tab_print(indent);
            
        }
        
    }

    else if (expr->type == BinExprT) {
        //struct BinExpr *bin = ;

        tab_print(indent);
        printf("operator: %s (%s)\n", p_bin_operator_sym(expr->inner.bin.op), print_bin_operator(expr->inner.bin.op));
        tab_print(indent);

        
        printf("left: {\n");
        if (expr != expr->inner.bin.lhs) 
            print_expr(expr->inner.bin.lhs, indent+1);
        
        tab_print(indent);
        printf("},\n");

        tab_print(indent);
        printf("right: {\n");
        print_expr(expr->inner.bin.rhs, indent+1);
        tab_print(indent);
        printf("}\n");
    }

    else {
        printf("\n===============\nERROR \n===============\n");
        return -1;
    }

    return 0;
}

#define tb "├"
#define rb "─"
#define fb "└"

void small_tab(short unsigned indent) {
    for (int i = 0; indent > i; i++) 
        printf("  ");
}

void ptree_inner(Expr *expr, short unsigned indent){
    if (expr->type == BinExprT) {
        printf("%s ", p_bin_operator_sym(expr->inner.bin.op));
        tab_print(5-indent);
        printf("\t(%d) \n", expr->depth);
        small_tab(indent);
        printf("├─");
        ptree_inner(expr->inner.bin.lhs, indent+1);
        printf("\n");
        small_tab(indent);
        printf("└─");
        ptree_inner(expr->inner.bin.rhs, indent+1);
        
    }
    else if (expr->type == UniExprT) {
        if (expr->inner.uni.op == UniValue) {
            if (expr->inner.uni.interal_data.symbol.tag == ValueTag) {
                if (expr->inner.uni.interal_data.symbol.inner.value.type == IntT) {
                    printf("%d", expr->inner.uni.interal_data.symbol.inner.value.data.integer);
                }
                else if (expr->inner.uni.interal_data.symbol.inner.value.type == StringT) {
                    printf("STR");
                }
            }
            else if (expr->inner.uni.interal_data.symbol.tag == VariableTag) {
                printf("%s", expr->inner.uni.interal_data.symbol.inner.variable);
            }
        }

        else if (expr->inner.uni.op == UniCall) {
            printf("FCALLS UNSUPPORTED");
        }
    }
}


void ptree(Expr *expr) {
    ptree_inner(expr, 0);
    printf("\n");
}