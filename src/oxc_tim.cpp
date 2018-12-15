#include <oxc_tim.h>

#ifdef USE_OXC_DEBUG
  #include <oxc_devio.h>
  #include <oxc_outstream.h>
#endif


uint32_t get_TIM_in_freq( TIM_TypeDef *tim )
{
  #ifdef APB2PERIPH_BASE
  if( (uint32_t)(tim) >= APB2PERIPH_BASE ) { // TIM1, TIM8
    uint32_t hclk  = HAL_RCC_GetHCLKFreq();
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
    if( hclk != pclk2 ) { // *2 : if APB2 prescaler != 1 (=2)
      pclk2 *= 2;
    }
    return pclk2;
  }
  #endif
  uint32_t hclk  = HAL_RCC_GetHCLKFreq();
  uint32_t pclk1 = HAL_RCC_GetPCLK1Freq();
  if( hclk != pclk1 ) { // *2 : if APB1 prescaler != 1 (=2)
    pclk1 *= 2;
  }
  return pclk1;
}

uint32_t get_TIM_cnt_freq( TIM_TypeDef *tim )
{
  uint32_t freq = get_TIM_in_freq( tim ); // in_freq
  uint32_t psc = 1 + tim->PSC;
  return freq / psc;
}

uint32_t get_TIM_base_freq( TIM_TypeDef *tim )
{
  uint32_t freq = get_TIM_cnt_freq( tim );
  uint32_t arr = 1 + tim->ARR;
  return freq / arr;
}

uint32_t calc_TIM_psc_for_cnt_freq( TIM_TypeDef *tim, uint32_t cnt_freq )
{
  uint32_t freq = get_TIM_in_freq( tim ); // in_freq
  uint32_t psc = freq / cnt_freq - 1;
  return psc;
}

uint32_t calc_TIM_arr_for_base_freq( TIM_TypeDef *tim, uint32_t base_freq )
{
  uint32_t freq = get_TIM_cnt_freq( tim ); // cnf_freq
  uint32_t arr = freq / base_freq - 1;
  return arr;
}

#ifdef USE_OXC_DEBUG
void tim_print_cfg( TIM_TypeDef *tim )
{
  STDOUT_os;
  if( !tim ) {
    os << NL << __PRETTY_FUNCTION__  << " : tim = nullptr";
    return;
  }
  uint32_t arr = tim->ARR;
  uint32_t psc = tim->PSC;
  uint32_t freq_in = get_TIM_in_freq( tim );

  int freq1 = freq_in  / ( psc + 1 );
  int freq2 = freq1    / ( arr + 1 );
  os <<  "# timer: f_in: "  <<  freq_in
     <<  " PSC: "   <<  psc
     <<  " ARR: "   <<  arr
     <<  " freq1: " <<  freq1
     <<  " freq2: " <<  freq2
     <<  " CNT: "   <<  tim->CNT
     <<  " CCR1: "  <<  tim->CCR1
     <<  " CR1: "   <<  HexInt( tim->CR1, true )
     <<  " CR2: "   <<  HexInt( tim->CR2, true )
     <<  " SMCR: "  <<  HexInt( tim->SMCR )
     <<  " DIER: "  <<  HexInt( tim->DIER )
     <<  " SR: "    <<  HexInt( tim->SR )
     << NL;
}
#endif


