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
