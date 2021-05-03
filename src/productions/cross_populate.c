
#include "../parser/ast.h"


void infer_types(BlockStatement *ast) {
    for (size_t i=0; ast->length > i; i++) {
        if (ast->statements[i]->type == Declare) {

        }
    }

}