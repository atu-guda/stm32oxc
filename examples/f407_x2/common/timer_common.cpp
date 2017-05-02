#include <oxc_auto.h>

// for f407, ....
uint32_t get_TIM1_8_in_freq()
{
  uint32_t hclk  = HAL_RCC_GetHCLKFreq();
  uint32_t pclk2 = HAL_RCC_GetPCLK2Freq(); // for TIM1, 8
  if( hclk != pclk2 ) { // *2 : if APB2 prescaler != 1 (=2)
    pclk2 *= 2;
  }
  return pclk2;
}

