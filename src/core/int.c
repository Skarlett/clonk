#include <stdint.h>
#include <stdarg.h>

/*
  Add 2 unsigned 16bit integers within bounds
  of its MAX & MIN values.
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


uint16_t onkstd_sum_u16(uint16_t num[], uint16_t num_sz) {
  uint16_t sum = 0;
  for (uint16_t i = 0; num_sz > i; i++)
  {
    sum = onkstd_add_u16(sum, num[i]);

    if(sum == UINT16_MAX)
      break;
  }
  return sum;
}

uint16_t onkstd_mul_u16(uint16_t a, uint16_t b)
{

}
