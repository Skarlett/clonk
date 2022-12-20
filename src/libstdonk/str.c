#include "clonk.h"

#define ONK_ALLOC(type) malloc(sizeof(type))

struct onk_string {
    char * buf;
    uint16_t size;
    uint16_t capacity;
};


uint16_t onk_str_realloc_append(
    struct onk_string *dest, char * src
)
{
    uint16_t src_len = strlen(src);
    if(src_len > (dest->capacity - dest->size))
    {
        realloc();
    }

    memcpy(&dest->buf[dest->size], src, src_len);
}

uint16_t onk_str_append(
    struct onk_string *dest, char * src
)
{
    uint16_t src_len = strlen(src);
    if(src_len > (dest->capacity - dest->size))
        return 0;

    memcpy(&dest->buf[dest->size], src, src_len);
}
