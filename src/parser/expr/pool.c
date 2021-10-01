#include "../lexer/lexer.h"
#include "expr.h"
#include <stdint.h>
#include "../../prelude.h"

struct ExprPool {
    usize capacity;
    usize length;
    struct Expr *heap_buf;
};

const struct Expr * pool_get(
    const struct ExprPool * pool,
    unsigned long int idx
){
    return pool->heap_buf + (idx * sizeof(struct Expr));
}

uint32_t pool_push() {

}

uint32_t pool_pop() {

}