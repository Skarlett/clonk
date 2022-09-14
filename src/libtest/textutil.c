#include "clonk.h"
#include "libtest/masking.h"
#include <string.h>

/* int16_t onk_snprint_desc_tok( */
/*     char * buf, */
/*     uint16_t nbuf, */
/*     struct onk_desc_token_t *dtok, */
/*     uint16_t i, */
/*     uint16_t fmt_i */
/* ){ */

/*     char token[ONK_TOK_CHAR_SIZE]; */

/*     switch(dtok->slot_type) */
/*     { */
/*         case onk_static_slot: */
/*             onk_snprint_token_type( */
/*                 token, */
/*                 ONK_TOK_CHAR_SIZE, */
/*                 dtok->data.static_tok */
/*             ); */

/*         case onk_dynamic_slot: */
/*             onk_snprint_token_type( */
/*                 token, ONK_TOK_CHAR_SIZE, */
/*                 dtok->data.dyn_tok.arr[fmt_i] */
/*             ); */

/*         case onk_inspect_slot: */
/*             onk_snprint_token( */
/*                 token, nbuf, */
/*                 &dtok->data.inspect.token */
/*             ); */

/*         default: return -1; */
/*     } */
/* } */

/* int16_t _onk_snprint_full_desc_tok( */
/*     char * buf, uint16_t nbuf, */
/*     struct onk_desc_token_t *dtok) */
/* { */
/*     uint16_t remaining_bytes = nbuf; */
/*     const char *ptoken, *delim = " | "; */
/*     uint16_t ptok_len; */

/*     switch(dtok->slot_type) */
/*     { */
/*         case onk_static_slot: */
/*             return onk_snprintf_lex_arr_inner( */
/*                 buf, remaining_bytes, dtok->data.static_tok); */

/*         case onk_dynamic_slot: */
/*             for(uint16_t i=0; dtok->data.dyn_tok.narr; i++) */
/*             { */
/*                 ptoken = onk_ptoken(dtok->data.dyn_tok.arr[i]); */
/*                 ptok_len = strlen(ptoken); */

/*                 if(ptok_len >= remaining_bytes) */
/*                     return -1; */

/*                 strncat(buf, ptoken, remaining_bytes); */
/*                 remaining_bytes -= ptok_len; */

/*                 /\* on last iteration *\/ */
/*                 if(i == dtok->data.dyn_tok.narr - 1) */
/*                     strncat(buf, delim, remaining_bytes); */
/*                 else */
/*                     buf[(nbuf - remaining_bytes || 1) - 1] = ']'; */
/*             } */
/*             // TODO: fix me */
/*             break; */

/*         case onk_inspect_slot: */
/*             return onk_snprint_token( */
/*                 buf, remaining_bytes, */
/*                 &dtok->data.inspect.token); */
/*             break; */

/*         default: */
/*             return -1; */
/*     } */
/* } */


/* int16_t onk_snprint_descriptor_arr( */
/*     char * buf, uint16_t nbuf, */
/*     struct onk_desc_token_t *arr, uint16_t narr) */
/* { */
/*     for(uint16_t i=0; narr > i; i++) */
/*     { */

/*     } */
/* } */


/* int16_t _onk_snprint_underline_mismatch() */
/* { */

/* } */


/* int16_t onk_snprint_mismatch() */
/* { */

/* } */
