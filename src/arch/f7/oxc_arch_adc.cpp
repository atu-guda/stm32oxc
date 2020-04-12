#include <algorithm>

#include <oxc_auto.h> // output and debug
#include <oxc_adc.h>

const AdcSampleTimeInfo adc_arch_sampletimes[] = {
  { ADC_SAMPLETIME_3CYCLES    ,   30 },
  { ADC_SAMPLETIME_15CYCLES   ,  150 },
  { ADC_SAMPLETIME_28CYCLES   ,  380 },
  { ADC_SAMPLETIME_56CYCLES   ,  560 },
  { ADC_SAMPLETIME_84CYCLES   ,  840 },
  { ADC_SAMPLETIME_112CYCLES  , 1120 },
  { ADC_SAMPLETIME_144CYCLES  , 1140 },
  { ADC_SAMPLETIME_480CYCLES  , 4800 }
};



const unsigned adc_arch_sampletimes_n = std::size( adc_arch_sampletimes );


uint32_t ADC_getFreqIn( ADC_HandleTypeDef* /* hadc */ )
{

  return HAL_RCC_GetPCLK2Freq();
}


uint32_t ADC_calc_div( ADC_HandleTypeDef* hadc, uint32_t freq_max, uint32_t *div_val )
{
  if( !hadc ) {
    return 0xFFFFFFFF;
  }


  uint32_t freq_in = ADC_getFreqIn( hadc );
  uint32_t dm = ( freq_in + freq_max - 1 ) / freq_max;
  uint32_t div = 0;
  uint32_t bits = 0;


  if( dm <= 2 ) {
    bits = ADC_CLOCK_SYNC_PCLK_DIV2;
    div = 2;
  } else if( dm <= 4 ) {
    bits = ADC_CLOCK_SYNC_PCLK_DIV4;
    div = 4;
  } else if( dm <= 6 ) {
    bits = ADC_CLOCK_SYNC_PCLK_DIV6;
    div = 6;
  } else if( dm <= 6 ) {
    bits = ADC_CLOCK_SYNC_PCLK_DIV8;
    div = 8;
  } else  {
    div = 0;
    bits = 0xFFFFFFFF;
  }

  if( div_val ) {
    *div_val = div;
  }
  return bits;
}


uint32_t ADC_conv_time_tick( uint32_t s_idx, uint32_t n_ch, uint32_t n_bits )
{
  if( s_idx >= adc_arch_sampletimes_n || n_ch < 1 || n_ch > 20 || n_bits > 12 || n_bits < 8 ) {
    return 0xFFFFFFFF;
  }
  return n_ch * (  n_bits + ( adc_arch_sampletimes[s_idx].stime10 / 10 ) ); // TODO: check!
}

uint32_t ADC_calcfreq( ADC_HandleTypeDef* hadc, ADC_freq_info *fi )
{
  if( ! hadc || ! fi ) {
    return 0;
  }

  uint32_t freq;
  fi->div = fi->div1 = fi->div2 = 1;
  fi->devbits = 0;

  uint32_t cclock = LL_ADC_GetCommonClock( __LL_ADC_COMMON_INSTANCE( hadc->Instance ) );

  fi->freq_in = freq = HAL_RCC_GetPCLK2Freq();
  switch( cclock ) {
    case ADC_CLOCK_SYNC_PCLK_DIV2:
      fi->div1 = 2;
      break;
    case ADC_CLOCK_SYNC_PCLK_DIV4:
      fi->div1 = 4;
      break;
    case ADC_CLOCK_SYNC_PCLK_DIV6:
      fi->div1 = 6;
      break;
    case ADC_CLOCK_SYNC_PCLK_DIV8:
      fi->div1 = 8;
      break;
    default:
      fi->devbits |= 0x4000; // debug: unknown
      break;
  }
  freq /= fi->div1;

  fi->div = fi->div1 * fi->div2;
  fi->freq = freq;
  return freq;
}


// ---------------- ADC_Info arch-dependent functions ---------------------------------

void ADC_Info::pr_state() const
{
  std_out
     << "# ADC: SR= "  << HexInt( BOARD_ADC_DEFAULT_DEV->SR  )
     <<  "  CR1= "      << HexInt( BOARD_ADC_DEFAULT_DEV->CR1 )
     <<  "  CR2= "      << HexInt( BOARD_ADC_DEFAULT_DEV->CR2 )
     <<  "  SQR1= "    << HexInt( BOARD_ADC_DEFAULT_DEV->SQR1 )
     <<  NL;
  std_out << "# adc_clk= " << adc_clk << " end_dma= " << end_dma << " n_series= " << n_series
     << " n_good= " << n_good << " n_bad= " << n_bad
     << " last_end= " << last_end << " last_error= " << last_error << " dma_error= " << dma_error << NL
     << "# good_SR= " << HexInt( good_SR ) << " bad_SR= " << HexInt(bad_SR) << " last_SR= " << HexInt(last_SR)
     << " data= " << HexInt( (void*)(data), 1 )
     <<  NL;
}

uint32_t ADC_Info::init_adc_channels()
{
  static const constexpr uint32_t ranks[] = {  // TODO: LL macros?
     ADC_REGULAR_RANK_1,  ADC_REGULAR_RANK_2,  ADC_REGULAR_RANK_3,  ADC_REGULAR_RANK_4,  ADC_REGULAR_RANK_5,
     ADC_REGULAR_RANK_6,  ADC_REGULAR_RANK_7,  ADC_REGULAR_RANK_8,  ADC_REGULAR_RANK_9, ADC_REGULAR_RANK_10,
    ADC_REGULAR_RANK_11, ADC_REGULAR_RANK_12, ADC_REGULAR_RANK_13, ADC_REGULAR_RANK_14, ADC_REGULAR_RANK_15,
    ADC_REGULAR_RANK_16
  };

  if( !ch_info || n_ch_max < 1 || (size_t)n_ch_max >= std::size(ranks) ) {
    return 0;
  }

  ADC_ChannelConfTypeDef sConfig;
  sConfig.SamplingTime = sampl_cycl_common;

  uint32_t n = 0;

  for( decltype(+n_ch_max) i=0; i<n_ch_max; ++i ) {
    if( ch_info[i].pin_num > 15 ) {
      break;
    }

    sConfig.Channel      = ch_info[i].channel;
    sConfig.Rank         = ranks[i];

    if( HAL_ADC_ConfigChannel( &hadc, &sConfig ) != HAL_OK )  {
      return 0;
    }
    ++n;
  }
  return n;
}


uint32_t ADC_Info::prepare_base( uint32_t presc, uint32_t sampl_cycl, uint32_t resol )
{
  if( hadc.Instance == 0 || n_ch_max < 1 ) {
    return 0;
  }
  sampl_cycl_common = sampl_cycl;

  hadc.Init.ClockPrescaler           = presc;
  hadc.Init.Resolution               = resol;
  // hadc.Init.ScanConvMode             = ADC_SCAN_DISABLE;
  // hadc.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
  hadc.Init.ContinuousConvMode       = DISABLE;
  // hadc.Init.NbrOfConversion          = 1;
  hadc.Init.DiscontinuousConvMode    = DISABLE;
  hadc.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.DataAlign                = ADC_DATAALIGN_RIGHT;
  hadc.Init.DMAContinuousRequests    = DISABLE;
  return 1;
}

uint32_t ADC_Info::prepare_single_manual( uint32_t presc, uint32_t sampl_cycl, uint32_t resol )
{
  if( ! prepare_base( presc, sampl_cycl, resol ) ) {
    return 0;
  }

  hadc.Init.ScanConvMode             = ADC_SCAN_DISABLE;
  hadc.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
  hadc.Init.NbrOfConversion          = 1;
  hadc.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
  prepared = 1;
  return 1;
}


uint32_t ADC_Info::prepare_multi_ev( uint32_t n_ch, uint32_t presc, uint32_t sampl_cycl, uint32_t ev, uint32_t resol )
{
  if( n_ch > n_ch_max ) {
    n_ch = n_ch_max;
  }
  if( ! prepare_base( presc, sampl_cycl, resol ) ) {
    return 0;
  }

  #if defined(STM32F7)
    __HAL_RCC_DAC_CLK_ENABLE(); // !!!!!!!!!!!!! see errata - need for timer interaction
  #endif

  hadc.Init.ScanConvMode             = ADC_SCAN_ENABLE;
  hadc.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
  hadc.Init.NbrOfConversion          = n_ch;
  hadc.Init.ExternalTrigConv         = ev;
  if( ev == ADC_SOFTWARE_START ) { // softstart
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  } else {
    hadc.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  }
  prepared = 3;
  return 1;
}


uint32_t ADC_Info::prepare_multi_ev_n( uint32_t n_ch, uint32_t presc, uint32_t sampl_cycl, uint32_t ev, uint32_t resol )
{
  if( n_ch > n_ch_max ) {
    n_ch = n_ch_max;
  }
  if( ! prepare_base( presc, sampl_cycl, resol ) ) {
    return 0;
  }

  #if defined(STM32F7)
    __HAL_RCC_DAC_CLK_ENABLE(); // !!!!!!!!!!!!! see errata - need for timer interaction
  #endif

  hadc.Init.ScanConvMode             = ADC_SCAN_ENABLE;
  hadc.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
  hadc.Init.NbrOfConversion          = n_ch;
  hadc.Init.ExternalTrigConv         = ev;
  hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc.Init.DMAContinuousRequests    = ENABLE; // for double-buffer DMA
  prepared = 4;
  return 1;
}



uint32_t ADC_Info::init_common()
{
  if( hadc.Instance == 0 || ! prepared ) {
    errno = 3000;
    return 0;
  }

  if( HAL_ADC_Init( &hadc ) != HAL_OK ) {
    errno = 3001;
    return 0;
  }

  // no ADC multi-mode here

  uint32_t n_ch_init = init_adc_channels();
  if( n_ch_init != n_ch_max )  {
    errno = 3003;
    return 0;
  }

  // no calibration

  return 1;
}

int ADC_Info::DMA_reinit( uint32_t mode )
{
  // std_out << "# debug: DMA_reinit start" NL;
  hdma_adc.Instance                 = BOARD_ADC_DMA_INSTANCE;

  hdma_adc.Init.Channel             = BOARD_ADC_DMA_CHANNEL;
  hdma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_adc.Init.Mode                = mode; // DMA_NORMAL, DMA_CIRCULAR, DMA_PFCTRL, DMA_DOUBLE_BUFFER_M0, DMA_DOUBLE_BUFFER_M1
  hdma_adc.Init.Priority            = DMA_PRIORITY_HIGH; // DMA_PRIORITY_LOW, DMA_PRIORITY_MEDIUM, DMA_PRIORITY_HIGH, DMA_PRIORITY_VERY_HIGH
  hdma_adc.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  // hdma_adc.Init.FIFOThreshold    = DMA_FIFO_THRESHOLD_HALFFULL;
  // hdma_adc.Init.MemBurst         = DMA_MBURST_SINGLE;
  // hdma_adc.Init.PeriphBurst      = DMA_PBURST_SINGLE;

  HAL_DMA_DeInit( &hdma_adc );
  if( HAL_DMA_Init( &hdma_adc ) != HAL_OK ) {
    errno = 7777;
    return 0;
  }

  __HAL_LINKDMA( &hadc, DMA_Handle, hdma_adc );

  // std_out << "# debug: DMA_reinit end" NL;
  return 1;
}

void ADC_Info::convCpltCallback( ADC_HandleTypeDef *hadc )
{
  if( prepared != 4 ) {
    end_dma |= 1; // not always
  } else {
    if( adcdma_n_status.base == 0 ) {
      end_dma |= 1; // not always
    }
  }
  good_SR =  last_SR = hadc->Instance->SR;
  last_end = 1;
  last_error = 0;
  ++n_good;
}

void ADC_Info::convHalfCpltCallback( ADC_HandleTypeDef *hadc )
{
}

void ADC_Info::errorCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT0 );
  end_dma |= 2;
  bad_SR = last_SR = hadc->Instance->SR;
  // tim2_deinit();
  last_end  = 2;
  last_error = HAL_ADC_GetError( hadc );
  dma_error = hadc->DMA_Handle->ErrorCode;
  hadc->DMA_Handle->ErrorCode = 0;
  ++n_bad;
}

