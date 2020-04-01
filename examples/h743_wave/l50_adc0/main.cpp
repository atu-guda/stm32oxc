#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_adcdata.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test ADC on H7 in one-shot mode one channel" NL
 " var t - delay time in us" NL
 " var n - default number of measurements" NL
 " var s - sample time index" NL
 " var x - supress normal output" NL
 " var v - reference voltage in uV " NL;

struct ADC_freq_info {
  uint32_t freq_in; //* input frequency
  uint32_t freq;    //* working freq
  uint32_t div;     //* total divider
  uint32_t div1;    //* base divider
  uint32_t div2;    //* second divider
  uint32_t devbits; //* misc bits about device: 1: sync mode, 2: ADC_VER_V5_3, 4: REV_ID_Y
};

int adc_arch_init_exa_1ch_manual( uint32_t presc, uint32_t sampl_cycl );
ADC_HandleTypeDef hadc1;
int v_adc_ref = BOARD_ADC_COEFF; // in uV, measured before test

// TODO: to common header
struct AdcSampleTimeInfo {
  uint32_t code;
  uint32_t stime10; //* in 10 * ADC_Cycles
};
uint32_t ADC_calcfreq( ADC_HandleTypeDef* hadc, ADC_freq_info *fi );
uint32_t ADC_getFreqIn( ADC_HandleTypeDef* hadc );

// TODO: to arch dependent header
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
constexpr unsigned adc_arch_sampletimes_n = size(adc_arch_sampletimes);


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] - test ADC "  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};




int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100000;
  UVAR('n') = 20;
  UVAR('c') = 1; // number of channels - 1 for this program, but need for next test
  UVAR('s') = adc_arch_sampletimes_n;
  UVAR('v') = v_adc_ref;


  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
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


  std_out << "# *** aux_mul= " << (int)(auxmul) << " dm= " << dm << NL;

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


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  const int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t n_ch = UVAR('c'); // 1, but cat set more for test
  const uint32_t t_step = UVAR('t') / 1000;
  const uint32_t t_step_us = UVAR('t');
  const xfloat k_all = 1e-6f * UVAR('v') / BOARD_ADC_DEFAULT_MAX;

  // TODO: to clock config file
  const uint32_t adc_arch_clock_in = ADC_getFreqIn( &hadc1 );
  uint32_t s_div = 0;
  uint32_t div_bits = ADC_calc_div( &hadc1, BOARD_ADC_FREQ_MAX, &s_div );

  std_out <<  NL "# Test0: n= " << n << " n_ch= " << n_ch
    << " t= " << t_step << " ms, freq_in= " << adc_arch_clock_in
    << " freq_max= " << BOARD_ADC_FREQ_MAX << NL;

  if( s_div == 0  ||  div_bits == 0xFFFFFFFF ) {
    std_out << "# error: fail to calc divisor" NL;
    return 7;
  }
  const uint32_t adc_freq = adc_arch_clock_in / s_div;
  const uint32_t adc_arch_clock_ns = (unsigned)( ( 1000000000LL + adc_freq - 1 ) / adc_freq );
  std_out << "# div= " << s_div << " bits: " << HexInt( div_bits ) << " freq: " << adc_freq
          << " tau: " << adc_arch_clock_ns << NL;

  uint32_t unsigned stime_idx = ( (uint32_t)UVAR('s') < adc_arch_sampletimes_n ) ? UVAR('s') : (adc_arch_sampletimes_n - 1);
  uint32_t stime_ns =  adc_arch_clock_ns * ADC_conv_time_tick( stime_idx, n_ch, BOARD_ADC_DEFAULT_BITS );
  if( stime_ns == 0xFFFFFFFF ) {
    std_out << "# error: fail to calculate conversion time" NL;
    return 8;
  }

  if( t_step_us * 1000 <= stime_ns ) {
    std_out << "# warn: time step (" << t_step_us * 1000 << ") ns < conversion time (" << stime_ns << ") ns" NL;
  }

  std_out << "# stime_idx= " << stime_idx << " 10ticks= " << adc_arch_sampletimes[stime_idx].stime10
          << " stime_ns= "  << stime_ns  << " code= " <<  adc_arch_sampletimes[stime_idx].code << NL;

  if( ! adc_arch_init_exa_1ch_manual( div_bits, adc_arch_sampletimes[stime_idx].code ) ) {
    std_out << "# error: fail to init ADC: errno= " << errno << NL;
  }

  // or such
  ADC_freq_info fi;
  ADC_calcfreq( &hadc1, &fi );
  std_out << "# ADC: freq_in: " << fi.freq_in << " freq: " << fi.freq
          << " div: " << fi.div << " div1: " << fi.div1 << " div2: " << fi.div2
          << " bits: " << HexInt( fi.devbits ) << NL;

  StatIntData sdat( 1, k_all );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();

    if( HAL_ADC_Start( &hadc1 ) != HAL_OK )  {
      std_out << "# error:  !! ADC Start error" NL;
      break;
    }

    if( HAL_ADC_PollForConversion( &hadc1, 10 ) == HAL_OK ) {
      int v = HAL_ADC_GetValue( &hadc1 );
      xfloat vv = k_all * v;
      sdat.add( &v );

      if( UVAR('x') == 0 ) {
        std_out << FmtInt( i, 5 ) << ' ' << FmtInt( tcc - tm00, 8 )
                << ' ' << FmtInt( v, 5 ) << ' ' << XFmt( vv, cvtff_auto, 9, 6 ) << NL;
      }
    } else {
      std_out << "# warn: Fail to wait poll!" NL;
      break;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  sdat.calc();
  std_out << sdat << NL;

  if( UVAR('d') > 1 ) {
    dump32( ADC1, 0x200 );
  }


  // HAL_ADC_Stop( &hadc1 );

  return 0;
}

// TODO: if ! HAVE_FLOAT
//int vv = v * ( UVAR('v') / 100 ) / BOARD_ADC_DEFAULT_MAX; // 100 = 1000/10
//std_out << " v= " << v <<  " vv= " << FloatMult( vv, 4 );

uint32_t ADC_getFreqIn( ADC_HandleTypeDef* hadc )
{
  if( ! hadc ) {
    return 0;
  }
  if( ADC_IS_SYNCHRONOUS_CLOCK_MODE( hadc ) ) {
    return HAL_RCC_GetHCLKFreq();
  } // else  async mode
  return HAL_RCCEx_GetPeriphCLKFreq( RCC_PERIPHCLK_ADC );
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

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

