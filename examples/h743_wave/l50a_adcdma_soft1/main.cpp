#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_adcdata.h>
#include <oxc_adcdata_cmds.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test ADC on H7 in manual shot N channels" NL
 " var t - delay time in us" NL
 " var n - default number of measurements" NL
 " var s - sample time index" NL
 " var c - number of channels" NL
 " var b - supress normal output" NL
 " var v - reference voltage in uV " NL;

const AdcChannelInfo adc_channels[] = {
  { BOARD_ADC_DEFAULT_CH0, BOARD_ADC_DEFAULT_GPIO0, BOARD_ADC_DEFAULT_PIN0 },
  { BOARD_ADC_DEFAULT_CH1, BOARD_ADC_DEFAULT_GPIO1, BOARD_ADC_DEFAULT_PIN1 },
  { BOARD_ADC_DEFAULT_CH2, BOARD_ADC_DEFAULT_GPIO2, BOARD_ADC_DEFAULT_PIN2 },
  { BOARD_ADC_DEFAULT_CH3, BOARD_ADC_DEFAULT_GPIO3, BOARD_ADC_DEFAULT_PIN3 },
  {                     0,                   GpioA,                    255 } // END
};

using AdcDataX = AdcData<BOARD_ADC_DEFAULT_BITS,xfloat>;
AdcDataX adcd( BOARD_ADC_MALLOC, BOARD_ADC_FREE );

ADC_Info adc( BOARD_ADC_DEFAULT_DEV, adc_channels );
// uint16_t ADC_buf[32];

int v_adc_ref = BOARD_ADC_COEFF; // in uV, measured before test


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " [n] - test ADC "  };
int cmd_set_coeffs( int argc, const char * const * argv );
CmdInfo CMDINFO_SET_COEFFS { "set_coeffs", 'F', cmd_set_coeffs, " k0 k1 k2 k3 - set ADC coeffs"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SET_COEFFS,
  nullptr
};



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100000;
  UVAR('n') = 20;
  UVAR('c') = adc.n_ch_max; // number of channels
  UVAR('s') = adc_arch_sampletimes_n - 1;
  UVAR('v') = v_adc_ref;

  // TODO: check
  #ifdef PWR_CR1_ADCDC1
  // PWR->CR1 |= PWR_CR1_ADCDC1;
  #endif

  BOARD_POST_INIT_BLINK;

  std_out << NL "##################### " PROJ_NAME NL;

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}



// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  const int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  const uint32_t n_ch = clamp<uint32_t>( UVAR('c'), 1, adc.n_ch_max );
  const uint32_t t_step = UVAR('t') / 1000;
  const uint32_t t_step_us = UVAR('t');
  const xfloat k_all = 1e-6f * UVAR('v') / BOARD_ADC_DEFAULT_MAX;

  const uint32_t adc_arch_clock_in = ADC_getFreqIn( &adc.hadc );
  uint32_t s_div = 0;
  uint32_t div_bits = ADC_calc_div( &adc.hadc, BOARD_ADC_FREQ_MAX, &s_div );

  // adc.set_channels( adc_channels );

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

  adc.prepare_multi_softstart( n_ch, div_bits, adc_arch_sampletimes[stime_idx].code, BOARD_ADC_DEFAULT_RESOLUTION );

  if( ! adc.init_xxx1() ) {
    std_out << "# error: fail to init ADC: errno= " << errno << NL;
  }

  if( UVAR('d') > 1 ) {
    dump32( BOARD_ADC_DEFAULT_DEV, 0x100 );
  }

  // or such
  ADC_freq_info fi;
  ADC_calcfreq( &adc.hadc, &fi );
  std_out << "# ADC: freq_in: " << fi.freq_in << " freq: " << fi.freq
          << " div: " << fi.div << " div1: " << fi.div1 << " div2: " << fi.div2
          << " bits: " << HexInt( fi.devbits ) << NL;


  // really need for H7 - DMA not work with ordinary memory
  adcd.free();
  if( ! adcd.alloc( n_ch, 1 ) ) {
    std_out << "# Error: fail to alloc buffer" << NL;
    return 2;
  }
  adc.data = adcd.data();
  adc.reset_cnt();
  adcd.set_d_t( t_step_us * 1e-6f );
  adcd.set_v_ref_uV( UVAR('v') );
  std_out << "# n_col= " << adcd.get_n_col() << " n_row= " << adcd.get_n_row() << " data: " << HexInt(adcd.data()) << " size_all= " << adcd.size_all() << NL;

  StatIntData sdat( n_ch, k_all );
  sdat.setScalesFrom( adcd );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR('b');


  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }
    float tc = 0.001f * ( tcc - tm00 );

    if( UVAR('l') ) {  leds.set( BIT2 ); }
    uint32_t r = adc.start_DMA_wait_1row( adcd.data(), n_ch );
    if( UVAR('l') ) {  leds.reset( BIT2 ); }

    if( r != 0 ) {
      std_out <<  "# error: start_DMA_wait_1row " << r << NL;
      rc = 1;
      break;
    }

    sdat.add( adcd.row(0) );

    if( do_out ) {
      adcd.out_float_row( std_out, tc, 0 );
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  sdat.calc();
  std_out << sdat << NL;

  if( UVAR('d') > 1 ) {
    dump32( BOARD_ADC_DEFAULT_DEV, 0x200 );
  }

  return rc;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != BOARD_ADC_DEFAULT_DEV ) {
    return;
  }
  // std_out << "# debug: " << __PRETTY_FUNCTION__ << NL;
  BOARD_ADC_DEFAULT_EN;
  BOARD_ADC_DMA_DEFAULT_EN;

  adc.init_gpio_channels();

  // here?
  adc.DMA_reinit( DMA_NORMAL );

  HAL_NVIC_SetPriority( BOARD_ADC_DMA_IRQ, 2, 0 );
  HAL_NVIC_EnableIRQ(   BOARD_ADC_DMA_IRQ );

  // HAL_NVIC_SetPriority( BOARD_ADC_IRQ, 3, 0 );
  // HAL_NVIC_EnableIRQ( BOARD_ADC_IRQ );
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != BOARD_ADC_DEFAULT_DEV ) {
    return;
  }
  // std_out << "# debug: " << __PRETTY_FUNCTION__ << NL;

  BOARD_ADC_DMA_DEFAULT_DIS;
  BOARD_ADC_DEFAULT_DIS;
}

void HAL_ADC_ConvHalfCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT1 );
  // adc.convHalfCpltCallback( hadc );
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT1 );
  adc.convCpltCallback( hadc );
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  // leds.set( BIT0 );
  adc.errorCallback( hadc );
}

void BOARD_ADC_DMA_IRQHANDLER(void)
{
  // leds.set( BIT2 );
  HAL_DMA_IRQHandler( &adc.hdma_adc );
}


// not used in single DMA
// void BOARD_ADC_IRQHANDLER(void)
// {
//   HAL_ADC_IRQHandler( &adc.hadc );
//   leds.toggle( BIT0 );
// }

int cmd_set_coeffs( int argc, const char * const * argv )
{
  return subcmd_set_coeffs( argc, argv, adcd );
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

