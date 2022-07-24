#include "predict.h"
#include "private.h"

void vframe_add_slice(
    struct validator_frame_t *f,
    const enum onk_lexicon_t *slice,
    uint16_t len
){
    f->slices[f->nslices] = slice;
    f->islices[f->nslices] = len;
    f->nslices += 1;
}

enum onk_lexicon_t vframe_add_delimiter(struct onk_parser_state_t * state)
{
  struct onk_parse_group_t *ghead = group_head(state);
  const struct onk_token_t *gmod;
  // wtf?
  //const struct onk_token_t *gmod = group_modifier(state, ghead);

  switch (ghead->type) {
    /* case onk_idx_op_token: */
    /*   if(ghead->delimiter_cnt > 2) */
    /*     return 0; */
    /*   return ONK_COMMA_TOKEN; */

    case onk_list_group_token:
      gmod = group_modifier(state, ghead);

      if (gmod && gmod->type == onk_idx_op_token)
      {
        if (2 > ghead->delimiter_cnt)
          return ONK_COLON_TOKEN;
        else return 0;
      }
      else return ONK_COMMA_TOKEN;


        return




    case onk_struct_group_token:
      return ONK_COMMA_TOKEN;

    case onk_tuple_group_token:
      return ONK_COMMA_TOKEN;

    case onk_map_group_token:
      if(ghead->delimiter_cnt % 2 == 0)
        return ONK_COLON_TOKEN;
      return ONK_COMMA_TOKEN;

    case onk_code_group_token:
      return ONK_SEMICOLON_TOKEN;

    default:
      return 0;
  }
}
