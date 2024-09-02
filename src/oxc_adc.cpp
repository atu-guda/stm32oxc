#include <oxc_adc.h>
#include <oxc_devio.h>
#include <oxc_outstream.h>


AdcDma_n_status adcdma_n_status;

ADC_Info::ADC_Info( ADC_TypeDef* hadc_, const AdcChannelInfo *ch_i )
{
  hadc.Instance  = hadc_;
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
    if( ch_i[n_ch_max].pin_num > 15 ) {
      break;
    }
  }
  return n_ch_max;
}

void ADC_Info::reset_cnt()
{
  end_dma = 0;
  dma_error = 0;
  n_series = 0;
  n_good = 0;
  n_bad = 0;
  last_SR = good_SR = bad_SR = last_end = last_error = last_status = 0;
}

uint32_t ADC_Info::init_gpio_channels()
{
  if( !ch_info || n_ch_max < 1 ) {
    return 0;
  }

  unsigned n = 0;
  for( decltype(+n_ch_max) i=0; i<n_ch_max; ++i ) {
    if( ch_info[i].pin_num > 15 ) {
      break;
    }
    ch_info[i].gpio.enableClk();
    ch_info[i].gpio.cfgAnalog( ch_info[i].pin_num );
    ++n;
  }
  return n;

}

bool ADC_Info::poll_and_read( int &v )
{
  if( int sta = HAL_ADC_PollForConversion( &hadc, 10 ); sta != HAL_OK ) {
    last_status = sta;
    return false;
  }
  v = HAL_ADC_GetValue( &hadc );
  return true;
}


uint32_t ADC_Info::start_DMA_wait( uint32_t n_ch, uint32_t n, uint32_t t_wait ) // 0 - ok
{
  if( ! data || n_ch < 1 || n_ch > n_ch_max || ! prepared || n < 1 ) {
    return 1;
  }

  end_dma = 0;

  if( int sta = HAL_ADC_Start_DMA( &hadc, (uint32_t*)(data), n_ch * n ); sta != HAL_OK )   {
    last_status = sta;
    // std_out <<  "# error: ADC_Start_DMA error " << sta << NL;
    return 2;
  }

  for( uint32_t ti=0; end_dma == 0 && ti < t_wait + 1000 && !break_flag;  ++ti ) {
    delay_ms( 1 );
  }

  HAL_ADC_Stop_DMA( &hadc ); // needed

  if( end_dma == 0 ) {
    // std_out <<  "# error: Fail to wait DMA end " NL;
    return 3;
  }

  if( dma_error != 0 ) {
    // std_out <<  "# error: Found DMA error " << HexInt( adc.dma_error ) <<  NL;
    return 4;
  }
  n_series = n;

  return 0;
}

uint32_t ADC_Info::start_DMA_wait_n( uint32_t n_ch, uint32_t n, uint32_t t_wait, uint32_t chunk_sz ) // 0 - ok
{
  if( ! data || n_ch < 1 || n_ch > n_ch_max || ! prepared || n < 1 ) {
    return 1;
  }

  end_dma = 0;

  if( int sta = ADC_Start_DMA_n( &hadc, (uint32_t*)(data), n_ch * n, chunk_sz ); sta != HAL_OK )   {
    last_status = sta;
    // std_out <<  "# error: ADC_Start_DMA error " << sta << NL;
    return 2;
  }

  for( uint32_t ti=0; end_dma == 0 && ti < t_wait + 1000 && !break_flag;  ++ti ) {
    delay_ms( 1 );
  }

  HAL_ADC_Stop_DMA( &hadc ); // needed

  if( end_dma == 0 ) {
    // std_out <<  "# error: Fail to wait DMA end " NL;
    return 3;
  }

  if( dma_error != 0 ) {
    // std_out <<  "# error: Found DMA error " << HexInt( adc.dma_error ) <<  NL;
    return 4;
  }
  n_series = n;

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc
