/*
** API functions for building test suites with tokens.
*/

#include "onkstd/int.h"
#include "clonk.h"
#include "libtest/masking.h"

/*
 * copies (`enum onk_lexicon_t *tok`) each item into
 * `onk_test_mask_t` as a static option.
*/
int8_t onk_desc_add_static_slot(
    struct onk_test_mask_t * mold,
    enum onk_lexicon_t *tok,
    uint16_t nitems)
{
    struct onk_desc_token_t dtok;

    if(mold->sarr > mold->narr)
        return -1;

    for(uint16_t i=0; nitems > i; i++)
    {
        if(tok[i] == 0)
            break;

        dtok.slot_type = onk_static_slot;
        dtok.data.static_tok = tok[i];

        mold->arr[mold->narr] = dtok;
        mold->narr += 1;
    }

    return 1;
}

int8_t onk_desc_add_inspect_slot(
    struct onk_test_mask_t *kit,
    struct onk_desc_inspect_token_t *inspect
){
    struct onk_desc_token_t descriptor;

    if(kit->narr >= kit->sarr)
      return -1;

    descriptor.slot_type = onk_inspect_slot;
    descriptor.data.inspect = *inspect;

    kit->arr[kit->narr] = descriptor;
    kit->narr += 1;
    return 0;
}

int onk_desc_add_dynamic_slot(
    struct onk_test_mask_t * kit,
    enum onk_lexicon_t *answers,
    uint16_t nanswers)
{
    struct onk_desc_token_t *dtok;

    if(kit->sarr > kit->narr)
        return -1;

    dtok = &kit->arr[kit->narr];
    kit->narr += 1;

    dtok->data.dyn_tok.arr = answers;
    dtok->data.dyn_tok.narr = nanswers;

    return 1;
}

int8_t onk_desc_add_static_repeating_slot(
    struct onk_test_mask_t *kit,
    enum onk_lexicon_t tok,
    uint16_t ntimes
)
{
    bool overflow = false;
    struct onk_desc_token_t descriptor;
    struct onk_desc_token_t *ptr;

    if(onk_add_u16(kit->narr, ntimes, &overflow) >= kit->sarr
       && overflow == 0)
        return -1;

    descriptor.slot_type = onk_static_slot;
    descriptor.data.static_tok = tok;

    ptr = &kit->arr[kit->narr];
    kit->narr += ntimes;

    for(uint16_t i=0; ntimes > i; i++)
        ptr[i] = descriptor;

    return 1;
}

int8_t onk_desc_add_repeating_slot(
    struct onk_test_mask_t *kit,
    struct onk_desc_token_t *tok,
    uint16_t ntimes
)
{
    bool overflow = false;
    struct onk_desc_token_t *ptr;

    if(onk_add_u16(kit->narr, ntimes, &overflow) >= kit->sarr
       && overflow == 0)
        return -1;

    ptr = &kit->arr[kit->narr];
    kit->narr += ntimes;

    memcpy(
        &kit->arr[kit->narr],
        tok,
        sizeof(struct onk_desc_token_t) * ntimes
    );

    return 1;
}

void onk_desc_init(
    struct onk_test_mask_t *kit,
    struct onk_desc_token_t *buffer,
    uint16_t buffer_sz
)
{
    kit->arr = buffer;
    kit->narr = buffer_sz;
    kit->ignore_whitespace = 1;
    kit->ignore_global_scope = 1;
}
