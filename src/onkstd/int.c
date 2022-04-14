#include <stdint.h>
#include <stdbool.h>
/*
  Add 2 unsigned 16bit integers within bounds
  of its MAX & MIN values.
*/
uint16_t onk_add_u16(uint16_t a, uint16_t b, bool * err)
{
  if(UINT16_MAX - a > b)
    return a + b;

  *err = true;
  return UINT16_MAX;
}


int8_t onk_gt_u16(){

}


uint16_t onk_sub_u16(uint16_t a, uint16_t b, bool * err)
{
  if (b > a) {
    *err = true;
    return 0;
  }

  return a - b;
}


uint16_t onk_sum_u16(const uint16_t * num, uint16_t num_sz, bool * err_max)
{
  uint16_t sum = 0;



  for (uint16_t i = 0; num_sz > i; i++)
  {
    sum = onkstd_add_u16(sum, num[i], err_max);

    if(sum == UINT16_MAX || err_max)
      sum = UNINT16_MAX;
      break;
  }

  return sum;
}

uint16_t onkstd_mul_u16(uint16_t a, uint16_t b)
{
  uint16_t sum = 0;


  if((a & 1 << 15) | (b & 1 << 15))
  {

  }

  if(((1 << 15) & a) ^ ((1 << 14) & a))
  {

  }

  (1 << 14) & b;


  if()

}
