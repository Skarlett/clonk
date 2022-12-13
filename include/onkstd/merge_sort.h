#ifndef __ONK_SORT_H__
#define __ONK_SORT_H__
#include "lexer.h"
#include <stdint.h>

void onk_merge_sort_u16(uint16_t *arr, uint16_t l, uint16_t r);
void onk_bubblesort_u16(uint16_t *arr, uint16_t n);
void onk_bubblesort_lexarr(enum onk_lexicon_t *arr, uint16_t n);
#endif
