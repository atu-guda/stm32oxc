#include <algorithm>

#include <oxc_adc.h>

const AdcSampleTimeInfo adc_arch_sampletimes[] = {
  { ADC_SAMPLETIME_1CYCLE_5    ,   15 },
  { ADC_SAMPLETIME_2CYCLES_5   ,   25 },
  { ADC_SAMPLETIME_8CYCLES_5   ,   85 },
  { ADC_SAMPLETIME_16CYCLES_5  ,  165 },
  { ADC_SAMPLETIME_32CYCLES_5  ,  325 },
  { ADC_SAMPLETIME_64CYCLES_5  ,  645 },
  { ADC_SAMPLETIME_387CYCLES_5 , 3785 },
  { ADC_SAMPLETIME_810CYCLES_5 , 8105 }
};

const unsigned adc_arch_sampletimes_n = std::size( adc_arch_sampletimes );


uint32_t ADC_getFreqIn( ADC_HandleTypeDef* hadc )
{
  if( ! hadc ) {
    return 0;
  }

  if( ADC_IS_SYNCHRONOUS_CLOCK_MODE( hadc ) ) {
    return HAL_RCC_GetHCLKFreq();
  }

  // else  async mode
  return HAL_RCCEx_GetPeriphCLKFreq( RCC_PERIPHCLK_ADC );
}

uint32_t ADC_calc_div( ADC_HandleTypeDef* hadc, uint32_t freq_max, uint32_t *div_val )
{
  if( !hadc ) {
    return 0xFFFFFFFF;
  }
  // TODO: BUG: what about sync mode?

#if defined(ADC_VER_V5_3)
  bool auxmul = true;
#else /* not ADC_VER_V5_3 */
  bool auxmul = ( HAL_GetREVID() > REV_ID_Y );
#endif /* end not ADC_VER_V5_3 */


  uint32_t freq_in = ADC_getFreqIn( hadc );
  if( auxmul ) {
    freq_in /= 2;
  }
  uint32_t dm = ( freq_in + freq_max - 1 ) / freq_max;
  uint32_t div = 0;
  uint32_t bits = 0;


  if( dm <= 1 ) {
    bits = ADC_CLOCK_ASYNC_DIV1;
    div = 1;
  } else if( dm <= 2 ) {
    bits = ADC_CLOCK_ASYNC_DIV2;
    div = 2;
  } else if( dm <= 4 ) {
    bits = ADC_CLOCK_ASYNC_DIV4;
    div = 4;
  } else if( dm <= 6 ) {
    bits = ADC_CLOCK_ASYNC_DIV6;
    div = 6;
  } else if( dm <= 8 ) {
    bits = ADC_CLOCK_ASYNC_DIV8;
    div = 8;
  } else if( dm <= 10 ) {
    bits = ADC_CLOCK_ASYNC_DIV10;
    div = 10;
  } else if( dm <= 12 ) {
    bits = ADC_CLOCK_ASYNC_DIV12;
    div = 12;
  } else if( dm <= 16 ) {
    bits = ADC_CLOCK_ASYNC_DIV16;
    div = 16;
  } else if( dm <= 32 ) {
    bits = ADC_CLOCK_ASYNC_DIV32;
    div = 32;
  } else if( dm <= 64 ) {
    bits = ADC_CLOCK_ASYNC_DIV64;
    div = 64;
  } else if ( dm <= 128 ) {
    bits = ADC_CLOCK_ASYNC_DIV128;
    div = 128;
  } else if( dm <= 256 ) {
    bits = ADC_CLOCK_ASYNC_DIV256;
    div = 256;
  } else {
    div = 0;
    bits = 0xFFFFFFFF;
  }

  if( auxmul ) {
    div *= 2;
  }

  if( div_val ) {
    *div_val = div;
  }
  return bits;
}


uint32_t ADC_conv_time_tick( uint32_t s_idx, uint32_t n_ch, uint32_t n_bits )
{
  if( s_idx >= adc_arch_sampletimes_n || n_ch < 1 || n_ch > 20 || n_bits > 16 || n_bits < 8 ) {
    return 0xFFFFFFFF;
  }
  return n_ch * ( ( 9 + ( adc_arch_sampletimes[s_idx].stime10 + 5 * n_bits ) ) / 10 );
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
  dbg_val1 = cclock; // TODO remove

  if( ADC_IS_SYNCHRONOUS_CLOCK_MODE( hadc ) ) {
    fi->devbits |= 1; // sync mode
    fi->freq_in = freq = HAL_RCC_GetHCLKFreq();
    switch( cclock ) {
      case ADC_CLOCK_SYNC_PCLK_DIV1:
      case ADC_CLOCK_SYNC_PCLK_DIV2:
        freq /= ( hadc->Init.ClockPrescaler >> ADC_CCR_CKMODE_Pos );
        fi->div1 = ( hadc->Init.ClockPrescaler >> ADC_CCR_CKMODE_Pos );
        break;
      case ADC_CLOCK_SYNC_PCLK_DIV4:
        freq /= 4UL;
        fi->div1 = 4;
        break;
      default:
        break;
    }
  } else { // async mode
    fi->freq_in = freq = HAL_RCCEx_GetPeriphCLKFreq( RCC_PERIPHCLK_ADC );
    fi->devbits |= 0x0100; // debug: async
    switch( cclock ) {
      case ADC_CLOCK_ASYNC_DIV2:
      case ADC_CLOCK_ASYNC_DIV4:
      case ADC_CLOCK_ASYNC_DIV6:
      case ADC_CLOCK_ASYNC_DIV8:
      case ADC_CLOCK_ASYNC_DIV10:
      case ADC_CLOCK_ASYNC_DIV12:
        fi->div1 = ( hadc->Init.ClockPrescaler >> ADC_CCR_PRESC_Pos ) << 1;
        fi->devbits |= 0x8000; // debug : 2-12
        break;
      case ADC_CLOCK_ASYNC_DIV16:
        fi->div1 = 16;
        break;
      case ADC_CLOCK_ASYNC_DIV32:
        fi->div1 = 32;
        break;
      case ADC_CLOCK_ASYNC_DIV64:
        fi->div1 = 64;
        break;
      case ADC_CLOCK_ASYNC_DIV128:
        fi->div1 = 128;
        break;
      case ADC_CLOCK_ASYNC_DIV256:
        fi->div1 = 256;
        break;
      default:
        fi->devbits |= 0x4000; // debug: unknown
        break;
    }
    freq /= fi->div1;
  }

#if defined(ADC_VER_V5_3)
  fi->devbits |= 2;
  freq /= 2U;
  fi->div2 *= 2;

#else /* not ADC_VER_V5_3 */
  if( HAL_GetREVID() <= REV_ID_Y ) {  /* STM32H7 silicon Rev.Y */
    fi->devbits |= 4;
  } else { /* STM32H7 silicon Rev.V */
    freq /= 2U; /* divider by 2 for Rev.V */
    fi->div2 *= 2;
  }
#endif /* end not ADC_VER_V5_3 */

  fi->div = fi->div1 * fi->div2;
  fi->freq = freq;
  return freq;
}


// ---------------- ADC_Info arch-dependent functions ---------------------------------

uint32_t ADC_Info::init_adc_channels()
{
  static const constexpr uint32_t ranks[] = {
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
  sConfig.SingleDiff   = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset       = 0;

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


uint32_t ADC_Info::prepare_single_manual( uint32_t presc, uint32_t sampl_cycl, uint32_t resol )
{
  if( hadc.Instance == 0 || n_ch_max < 1 ) {
    return 0;
  }
  sampl_cycl_common = sampl_cycl;

  hadc.Init.ClockPrescaler           = presc;
  hadc.Init.Resolution               = resol;
  hadc.Init.ScanConvMode             = ADC_SCAN_DISABLE;
  hadc.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
  hadc.Init.LowPowerAutoWait         = DISABLE;
  hadc.Init.ContinuousConvMode       = DISABLE;
  hadc.Init.NbrOfConversion          = 1;
  hadc.Init.DiscontinuousConvMode    = DISABLE;
  hadc.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
  hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
  hadc.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
  hadc.Init.OversamplingMode         = DISABLE;
  prepared = 1;
  return 1;
}


uint32_t ADC_Info::prepare_multi_ev( uint32_t n_ch, uint32_t presc, uint32_t sampl_cycl, uint32_t ev, uint32_t resol )
{
  if( n_ch > n_ch_max ) {
    n_ch = n_ch_max;
  }
  if( hadc.Instance == 0 || n_ch < 1 ) {
    return 0;
  }
  sampl_cycl_common = sampl_cycl;

  hadc.Init.ClockPrescaler           = presc;
  hadc.Init.Resolution               = resol;
  hadc.Init.ScanConvMode             = ADC_SCAN_ENABLE;
  hadc.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait         = DISABLE;
  hadc.Init.ContinuousConvMode       = DISABLE;
  hadc.Init.NbrOfConversion          = n_ch;
  hadc.Init.DiscontinuousConvMode    = DISABLE;
  hadc.Init.ExternalTrigConv         = ev;
  hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_ONESHOT; // ADC_CONVERSIONDATA_DR;
  hadc.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
  hadc.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
  hadc.Init.OversamplingMode         = DISABLE;
  prepared = 3;
  return 1;
}


uint32_t ADC_Info::prepare_multi_ev_n( uint32_t n_ch, uint32_t presc, uint32_t sampl_cycl, uint32_t ev, uint32_t resol )
{
  if( n_ch > n_ch_max ) {
    n_ch = n_ch_max;
  }
  if( hadc.Instance == 0 || n_ch < 1 ) {
    return 0;
  }
  sampl_cycl_common = sampl_cycl;

  hadc.Init.ClockPrescaler           = presc;
  hadc.Init.Resolution               = resol;
  hadc.Init.ScanConvMode             = ADC_SCAN_ENABLE;
  hadc.Init.EOCSelection             = ADC_EOC_SEQ_CONV;
  hadc.Init.LowPowerAutoWait         = DISABLE;
  hadc.Init.ContinuousConvMode       = DISABLE;
  hadc.Init.NbrOfConversion          = n_ch;
  hadc.Init.DiscontinuousConvMode    = DISABLE;
  hadc.Init.ExternalTrigConv         = ev;
  hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DMA_CIRCULAR; //
  hadc.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
  hadc.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
  hadc.Init.OversamplingMode         = DISABLE;
  prepared = 4;
  return 1;
}



uint32_t ADC_Info::init_xxx1()
{
  if( hadc.Instance == 0 || ! prepared ) {
    errno = 3000;
    return 0;
  }

  if( HAL_ADC_Init( &hadc ) != HAL_OK ) {
    errno = 3001;
    return 0;
  }

  //* no ADC multi-mode
  ADC_MultiModeTypeDef multimode;
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if( HAL_ADCEx_MultiModeConfigChannel( &hadc, &multimode ) != HAL_OK ) {
    errno = 3002;
    return 0;
  }

  uint32_t n_ch_init = init_adc_channels();
  // dbg_val0 = n_ch_init;
  if( n_ch_init != n_ch_max )  {
    errno = 3003;
    return 0;
  }

  if( HAL_ADCEx_Calibration_Start( &hadc, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED ) != HAL_OK ) {
    errno = 3010;
    return 0;
  }

  return 1;
}

int ADC_Info::DMA_reinit( uint32_t mode )
{
  // std_out << "# debug: DMA_reinit start" NL;
  hdma_adc.Instance                 = BOARD_ADC_DMA_INSTANCE;

  #ifdef BOARD_ADC_DMA_REQUEST
  hdma_adc.Init.Request             = BOARD_ADC_DMA_REQUEST;
  #endif
  #ifdef BOARD_ADC_DMA_CHANNEL
  hdma_adc.Init.Channel             = BOARD_ADC_DMA_CHANNEL;
  #endif

  hdma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_adc.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_adc.Init.Mode                = mode; // DMA_NORMAL, DMA_CIRCULAR, DMA_PFCTRL
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
  end_dma |= 1;
  good_SR =  last_SR = hadc->Instance->ISR;
  last_end = 1;
  last_error = 0;
  ++n_good;
}

void ADC_Info::convHalfCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT1 );
  /* Invalidate Data Cache to get the updated content of the SRAM on the first half of the ADC converted data buffer: 32 bytes */
  // SCB_InvalidateDCache_by_Addr((uint32_t *) &aADCxConvertedData[0], ADC_CONVERTED_DATA_BUFFER_SIZE);
}

void ADC_Info::errorCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT0 );
  end_dma |= 2;
  bad_SR = last_SR = hadc->Instance->ISR;
  // tim2_deinit();
  last_end  = 2;
  last_error = HAL_ADC_GetError( hadc );
  dma_error = hadc->DMA_Handle->ErrorCode;
  hadc->DMA_Handle->ErrorCode = 0;
  n_bad;
}

