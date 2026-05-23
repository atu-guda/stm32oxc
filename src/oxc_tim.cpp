#include <oxc_tim.h>

#ifdef USE_OXC_DEBUG
  #include <oxc_outstream.h>
#endif

reg32 oxc::TimCh::fake_ccr {0};


uint32_t get_TIM_in_freq( TIM_TypeDef *tim )
{
  #ifdef APB2PERIPH_BASE
  if( (uint32_t)(tim) >= APB2PERIPH_BASE ) { // TIM1, TIM8
    uint32_t hclk  = HAL_RCC_GetHCLKFreq();
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
    if( hclk != pclk2 ) { // *2 : if APB2 prescaler != 1 (=2)
      pclk2 *= 2;
    }
    // TODO: use switch from RCC
    #ifdef STM32F334
    if( tim == TIM1 ) {
      pclk2 *= 2;
    }
    #endif
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


uint32_t get_TIM_base_freq( TIM_TypeDef *tim )
{
  return get_TIM_cnt_freq( tim ) / ( 1 + tim->ARR );
}

uint32_t calc_TIM_psc_for_cnt_freq( TIM_TypeDef *tim, uint32_t cnt_freq )
{
  return  get_TIM_in_freq( tim ) / cnt_freq - 1;
}

uint32_t calc_TIM_arr_for_base_freq( TIM_TypeDef *tim, uint32_t base_freq )
{
  return get_TIM_cnt_freq( tim ) / base_freq - 1;
}

uint32_t calc_TIM_arr_for_base_psc( TIM_TypeDef *tim, uint32_t psc, uint32_t base_freq )
{
  uint32_t freq = get_TIM_in_freq( tim ) / ( 1 + psc );
  return freq / base_freq - 1;
}

#ifdef USE_OXC_DEBUG
void tim_print_cfg( TIM_TypeDef *tim )
{
  if( !tim ) {
    std_out << NL << __PRETTY_FUNCTION__  << " : tim = nullptr";
    return;
  }
  uint32_t arr = tim->ARR;
  uint32_t psc = tim->PSC;
  uint32_t freq_in = get_TIM_in_freq( tim );

  auto freq1 = get_TIM_cnt_freq( tim );
  auto freq2 = get_TIM_base_freq( tim );
  std_out <<  "# timer " << HexInt( tim )
     <<  " PSC: "   <<  psc
     <<  " ARR: "   <<  arr
     <<  " f_in: "  <<  freq_in
     <<  " cnt: " <<  freq1
     <<  " base: " <<  freq2
     <<  NL "# CR1: "   <<  HexInt( tim->CR1, true )
     <<  " CR2: "   <<  HexInt( tim->CR2, true )
     <<  " SMCR: "  <<  HexInt( tim->SMCR, true )
     <<  " DIER: "  <<  HexInt( tim->DIER, true )
     <<  NL "# SR: "    <<  HexInt( tim->SR, true )
     <<  " CNT: "   <<  tim->CNT
     <<  " CCR1: "  <<  tim->CCR1
     <<  " CCR2: "  <<  tim->CCR2
     << NL;
}
#endif


