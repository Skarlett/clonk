
#ifndef _HEADER_EXPR_POOL__
#define _HEADER_EXPR_POOL__

#include "../../prelude.h"
#include "expr.h"

#define CACHE_SZ 256

struct ExprPool {
    usize capacity;
    usize length;
    struct Expr *heap_buf;

    struct Expr *cached[CACHE_SZ];
    uint8_t cached_ctr;
};

const struct Expr * pool_get(const struct ExprPool * pool, usize idx);
struct Expr * pool_push(struct ExprPool *pool, const struct Expr e);
struct Expr * pool_pop(struct ExprPool *pool);
int8_t pool_realloc(struct ExprPool *pool);
#endif
