#include <oxc_adc.h>
#include <oxc_devio.h>
#include <oxc_outstream.h>

#if defined(STM32F4) || defined (STM32F7)

const uint32_t sampl_times_codes[adc_n_sampl_times] = { // all for 36 MHz ADC clock
  ADC_SAMPLETIME_3CYCLES   , //  15  tick: 2.40 MSa,  0.42 us
  ADC_SAMPLETIME_15CYCLES  , //  27  tick: 1.33 MSa,  0.75 us
  ADC_SAMPLETIME_28CYCLES  , //  40  tick:  900 kSa,  1.11 us
  ADC_SAMPLETIME_56CYCLES  , //  68  tick:  529 kSa,  1.89 us
  ADC_SAMPLETIME_84CYCLES  , //  96  tick:  375 kSa,  2.67 us
  ADC_SAMPLETIME_144CYCLES , // 156  tick:  231 kSa,  4.33 us
  ADC_SAMPLETIME_480CYCLES   // 492  tick:   73 kSa, 13.67 us
};

const uint32_t sampl_times_cycles[adc_n_sampl_times] = { // sample+conv(12)
    15,  // ADC_SAMPLETIME_3CYCLES
    27,  // ADC_SAMPLETIME_15CYCLES
    40,  // ADC_SAMPLETIME_28CYCLES
    68,  // ADC_SAMPLETIME_56CYCLES
    96,  // ADC_SAMPLETIME_84CYCLES
   156,  // ADC_SAMPLETIME_144CYCLES
   492,  // ADC_SAMPLETIME_480CYCLES
};

uint32_t calc_ADC_clk( uint32_t presc, int *div_val )
{
  int dv_fake = 0;
  if( div_val == nullptr ) {
    div_val = &dv_fake;
  }
  *div_val = 1;

  uint32_t clk =  HAL_RCC_GetPCLK2Freq();
  switch( presc ) {
    case ADC_CLOCK_SYNC_PCLK_DIV2: *div_val = 2; break;
    case ADC_CLOCK_SYNC_PCLK_DIV4: *div_val = 4; break;
    case ADC_CLOCK_SYNC_PCLK_DIV6: *div_val = 6; break;
    case ADC_CLOCK_SYNC_PCLK_DIV8: *div_val = 8; break;
    default: break; // newer
  }
  clk /= *div_val = 2;
  return clk;
}

uint32_t hint_ADC_presc()
{
  uint32_t clk =  HAL_RCC_GetPCLK2Freq();
  const uint32_t max_ADC_Clk = ADC_FREQ_MAX;
  if( ( clk / 2 ) <= max_ADC_Clk ) {
    return ADC_CLOCK_SYNC_PCLK_DIV2;
  }
  if( ( clk / 4 ) <= max_ADC_Clk ) {
    return ADC_CLOCK_SYNC_PCLK_DIV4;
  }
  if( ( clk / 6 ) <= max_ADC_Clk ) {
    return ADC_CLOCK_SYNC_PCLK_DIV6;
  }
  return ADC_CLOCK_SYNC_PCLK_DIV8;
}

void pr_ADC_state( const ADC_Info &adc )
{
  std_out << "# ADC: SR= " << HexInt( BOARD_ADC_DEFAULT_DEV->SR  )
     <<  "  CR1= "    << HexInt( BOARD_ADC_DEFAULT_DEV->CR1 )
     <<  "  CR2= "    << HexInt( BOARD_ADC_DEFAULT_DEV->CR2 )
     <<  "  SQR1= "   << HexInt( BOARD_ADC_DEFAULT_DEV->SQR1 )
     <<  "  SQR2= "   << HexInt( BOARD_ADC_DEFAULT_DEV->SQR2 )
     <<  "  SQR3= "   << HexInt( BOARD_ADC_DEFAULT_DEV->SQR3 )
     <<  NL;
  std_out << "# adc_clk= " << adc.adc_clk << " end_dma= " << adc.end_dma << " n_series= " << adc.n_series
     << " n_good= " << adc.n_good << " n_bad= " << adc.n_bad
     << " last_end= " << adc.last_end << " last_error= " << adc.last_error
     <<  NL;
}

#endif

ADC_Info::ADC_Info( ADC_TypeDef* _hadc, const AdcChannelInfo *ch_i )
{
  hadc.Instance  = _hadc;
  set_channels( ch_i );

  reset_cnt();
}

uint32_t ADC_Info::set_channels( const AdcChannelInfo *ch_i )
{
  n_ch_max = 0;
  ch_info = ch_i;
  if( ! ch_i ) {
    return 0;
  }

  for( ; ; ++n_ch_max ) {
    if( ch_i[n_ch_max].pin_num == 0 ) {
      break;
    }
  }
  return n_ch_max;
}

void ADC_Info::reset_cnt()
{
  // n_ch_max = 4;
  end_dma = 0;
  dma_error = 0;
  n_series = 0;
  // n_series_todo = 0;
  n_good = 0;
  n_bad = 0;
  last_SR = good_SR = bad_SR = last_end = last_error = 0;
}

uint32_t ADC_Info::init_gpio_channels()
{
  if( !ch_info || n_ch_max < 1 ) {
    return 0;
  }

  unsigned n = 0;
  for( int i=0; i<n_ch_max; ++i ) {
    if( ch_info[i].pin_num == 0 ) {
      break;
    }
    ch_info[i].gpio.enableClk();
    ch_info[i].gpio.cfgAnalog( ch_info[i].pin_num );
    ++n;
  }
  return n;

}

// vim: path=.,/usr/share/stm32cube/inc
