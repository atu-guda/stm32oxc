#include <oxc_auto.h>

// for f3xx, ....
uint32_t get_TIM1_8_in_freq()
{
  return 2 * HAL_RCC_GetHCLKFreq(); // really 2 * PLLCLK
}

