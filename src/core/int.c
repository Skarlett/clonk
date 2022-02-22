#include <stdint.h>

/*
  Add 2 unsigned 16bit integers within bounds
*/
uint16_t onkstd_add_u16(uint16_t a, uint16_t b)
{
  if (UINT16_MAX == a || UINT16_MAX == b)
    return UINT16_MAX;

  else if(UINT16_MAX - a > b)
    return a + b;

  return UINT16_MAX;
}

uint16_t onkstd_sub_u16(uint16_t a, uint16_t b)
{
  if (b > a) return 0;

  return a - b;
}
