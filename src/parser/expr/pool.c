#include "../../prelude.h"
#include "../lexer/lexer.h"
#include "expr.h"
#include "pool.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

const struct Expr * pool_get(const struct ExprPool * pool, usize idx)
{
    if (pool->capacity > idx)
        return 0;
    
    return pool->heap_buf + sizeof(struct Expr) * idx;
}

struct Expr * _pool_push(struct ExprPool *pool, const struct Expr e)
{
    if (pool->length >= pool->capacity)
    {
        /* is cache empty? */
        if (pool->cached_ctr == 0) {
            /* collect garbage/freed expression */
            for (usize i=0; pool->capacity > i; i++)
            {
                if (pool->cached_ctr >= (uint8_t)CACHE_SZ)
                    break;
                    
                else if (pool->heap_buf[i].free)
                {
                    pool->cached[pool->cached_ctr] = &pool->heap_buf[i];
                    pool->cached_ctr += 1;
                }
            }

            if (pool->cached_ctr > 0)
            {
                pool->cached_ctr -= 1;
                return pool->cached[pool->cached_ctr];
            }
            return 0;
        }

        /* give free'd expr*/
        else if (pool->cached_ctr > 0)
        {
            pool->cached_ctr -= 1;
            return pool->cached[pool->cached_ctr];
        }
    }
     
    else if (pool->capacity > pool->length){
        pool->heap_buf[pool->length] = e;
        pool->length += 1;
    
        return &pool->heap_buf[pool->length - 1];    
    }

    return 0;
}

struct Expr * pool_push(struct ExprPool *pool, const struct Expr ex)
{
    struct Expr *phandle;
    uint8_t phandle_i;

    do {
        phandle = _pool_push(pool, ex);
        if (pool_realloc(pool) == -1 || phandle_i > 1)
            return 0;
        phandle_i += 1;
    } while (phandle == 0);
    
    return phandle;
}

struct Expr * pool_pop(struct ExprPool *pool)
{
    if (0 >= pool->length)
        return 0;
    
    pool->length -= 1;
    pool->heap_buf[pool->length].free = true;
    
    return &pool->heap_buf[pool->length];
}


int8_t pool_realloc(struct ExprPool *pool) {
    struct Expr * old_heap;

    pool->capacity *= 2;
    old_heap = pool->heap_buf;

    pool->heap_buf = realloc(pool->heap_buf, pool->capacity * sizeof(struct Expr));

    if (pool->heap_buf == 0) {
        pool->heap_buf = old_heap;
        pool->capacity = pool->capacity / 2;
        return -1;
    }

    return 0;
}