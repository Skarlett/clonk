#include "clonk.h"


bool onk_can_add_u16(uint16_t a, uint16_t b);
uint16_t onk_add_u16(uint16_t a, uint16_t b, bool *overflow);
uint16_t onk_sub_u16(uint16_t a, uint16_t b, bool *overflow);
uint16_t onk_sum_u16(uint16_t num[], uint16_t num_sz, bool *overflow);
