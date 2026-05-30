#include <cmath>

#include <oxc_cpptypes.h>
#include <oxc_tim.h>

#ifdef USE_OXC_DEBUG
  #include <oxc_outstream.h>
#endif

using namespace oxc;

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

float get_TIM_base_freq_f( TIM_TypeDef *tim )
{
  return (float)get_TIM_in_freq( tim ) / ( ( 1+ tim->PSC ) *  (1 + tim->ARR) );
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

std::pair<uint32_t,uint32_t> calc_tim_psc_arr( float f_in, float f_out, uint32_t arr_min, uint32_t arr_max )
{
  const float k_f = (float)(f_in) / f_out;
  const float k_1 = k_f / (arr_max+1);
  if( k_1 > 65535 ) {
    return { 0xFFFFFFFF, 0 }; // bad
  }

  uint16_t psc_min = (uint16_t)( k_1 );
  float d_fa_min { 1e30f };
  uint32_t arr_e { 0 }, psc_e { 0xFFFFFFFF };
  float f_1 {0}, f_2 {0};

  for( uint32_t psc = psc_min; psc <= 0xFFFF; ++psc ) {
    f_1 = (float)f_in / ( psc + 1 );
    float k_2 = f_1 / f_out;
    uint32_t ki_2 = (uint32_t)( k_2 + 0.4999f );
    uint32_t arr = ki_2 - 1;
    if( arr < arr_min ) {
      break;
    }
    f_2 = f_1 / ( arr + 1 );
    float d_f  = f_out - f_2;
    float d_fa = std::fabsf( d_f );

    if( d_fa < d_fa_min ) {
      psc_e = psc; arr_e = arr;
      d_fa_min = d_fa;
    }
    // std_out << "# " << psc << ' ' << arr << ' ' << f_1 << ' ' << f_2 << ' ' << d_f  << ' ' << c << endl;
    if( d_fa < 1e-8f ) {
      break;
    }
  }
  return { psc_e, arr_e };
}

//  t_h.Instance must be set beforehand;
ReturnCode tim_pwm_cfg_default( TIM_HandleTypeDef &t_h, uint32_t psc, uint32_t arr, std::span<const TimChPin> chpins )
{
  if( t_h.Instance == nullptr ) {
    return rcFatal;
  }

  t_h.Init.Prescaler         = psc;
  t_h.Init.Period            = arr;
  t_h.Init.ClockDivision     = 0;
  t_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  t_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &t_h ) != HAL_OK ) {
    errno = 3000;
    return rcErr;
  }

  HAL_TIM_ConfigClockSource( &t_h, &sClockSourceConfig_def );

  if( HAL_TIMEx_MasterConfigSynchronization( &t_h, &sMasterConfig_def ) != HAL_OK ) {
    errno = 3001;
    return rcErr;
  }

  for( auto ch : chpins ) {
    if( HAL_TIM_PWM_ConfigChannel( &t_h, &tim_oc_cfg_default_pwm1, TimCh::ch2hal_ch(ch.ch) ) != HAL_OK ) {
      errno = 3002;
      return rcErr;
    }
    HAL_TIM_PWM_Start( &t_h, TimCh::ch2hal_ch(ch.ch) );
  }
  return rcOk;
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
     <<  " f_cnt: " <<  freq1
     <<  " f_base: " << freq2
     <<  NL
         "# CR1: "  <<  HexInt( tim->CR1,   true )
     <<  " CR2: "   <<  HexInt( tim->CR2,   true )
     <<  " SMCR: "  <<  HexInt( tim->SMCR,  true )
     <<  " DIER: "  <<  HexInt( tim->DIER,  true )
     <<  " CCMR1: " <<  HexInt( tim->CCMR1, true )
     <<  " CCMR2: " <<  HexInt( tim->CCMR2, true )
     <<  " CCER: "  <<  HexInt( tim->CCER,  true )
     <<  NL
         "# SR: "    << HexInt( tim->SR,    true )
     <<  " CNT: "   <<  tim->CNT
     <<  " CCR1: "  <<  tim->CCR1
     <<  " CCR2: "  <<  tim->CCR2
     << NL;
}
#else
void tim_print_cfg( TIM_TypeDef *tim )
{
}
#endif


