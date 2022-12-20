#include "../prelude.h"
#include <stdint.h>
#include <string.h>
#include "clonk.h"


struct ListItemHeader {
    uint16_t size;
    bool free;
};

struct List {
    void *base;
    void *head;

    usize last_sz;
    usize size;
    usize len;
};


void * list_push(struct List *list, void *src, usize size)
{
    void * ret;
    struct ListItemHeader item;
    item.size = size;
    item.free = false;
    
    // realloc
    if ((list->head - list->base) + sizeof(struct ListItemHeader) + size > list->size)
        return 0;

    list->head = memcpy(list->head, &item, sizeof(struct ListItemHeader));
    ret = list->head;
    list->head = memcpy(list->head, src, size);
    return ret;
}
