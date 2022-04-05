#include "clonk.h"
#include "libtest/tokens.h"

int16_t onk_snprint_desc_tok(
    char * buf, uint16_t nbuf,
    struct onk_desc_token_t *dtok) {
{

    switch(dtok->slot_type)
    {
        case onk_static_slot:
            onk_ptoken(dtok->data.static_tok);

            break;

        case onk_dynamic_slot:
            break;

        case onk_inspect_slot:
            break;

        default:
            return -1;
    }

}
