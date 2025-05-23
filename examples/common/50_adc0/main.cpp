#include <cerrno>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_adcdata.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test ADC in one-shot mode one channel" NL
 " var t - delay time in us" NL
 " var n - default number of measurements" NL
 " var s - sample time index" NL
 " var b - supress normal output" NL
 " var v - reference voltage in uV " NL;

const AdcChannelInfo adc_channels[] = {
  { BOARD_ADC_DEFAULT_CH0, BOARD_ADC_DEFAULT_GPIO0, BOARD_ADC_DEFAULT_PIN0 },
  {                     0,                   GpioA, AdcChannelInfo::pin_num_end } // END
};

ADC_Info adc( BOARD_ADC_DEFAULT_DEV, adc_channels );

int v_adc_ref = BOARD_ADC_COEFF; // in uV, measured before test


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
  UVAR('c') = adc.n_ch_max; // number of channels - 1 for this program, but need for next test
  UVAR('s') = adc_arch_sampletimes_n - 1;
  UVAR('v') = v_adc_ref;

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
  const uint32_t n_ch = clamp<uint32_t>( UVAR('c'), 1, adc.n_ch_max );
  //
  uint32_t unsigned stime_idx = ( (uint32_t)UVAR('s') < adc_arch_sampletimes_n ) ? UVAR('s') : (adc_arch_sampletimes_n - 1);
  const uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1 );

  const uint32_t t_step_us = UVAR('t');
  adc.t_step_f = (decltype(adc.t_step_f))(1e-6f) * t_step_us;
  const xfloat freq_sampl = (xfloat)1e6f / t_step_us;
  const uint32_t t_step = UVAR('t') / 1000;
  const xfloat k_all = 1e-6f * UVAR('v') / BOARD_ADC_DEFAULT_MAX;

  const uint32_t adc_arch_clock_in = ADC_getFreqIn( &adc.hadc );
  uint32_t s_div = 0;
  uint32_t div_bits = ADC_calc_div( &adc.hadc, ADC_FREQ_MAX, &s_div );

  std_out <<  NL "# Test0: n= " << n << " n_ch= " << n_ch
    << " t= " << t_step_us << " us, freq_sampl= " << freq_sampl
    << " freq_in= " << adc_arch_clock_in
    << " freq_max= " << ADC_FREQ_MAX << NL;

  delay_ms( 100 );

  if( s_div == 0  ||  div_bits == 0xFFFFFFFF ) {
    std_out << "# error: fail to calc divisor" NL;
    return 7;
  }
  const uint32_t adc_freq = adc_arch_clock_in / s_div;
  adc.adc_clk = adc_freq; // TODO: place in good? place
  const uint32_t adc_clock_ns = (unsigned)( ( 1000000000LL + adc_freq - 1 ) / adc_freq );
  std_out << "# div= " << s_div << " bits: " << HexInt( div_bits ) << " freq: " << adc_freq
          << " adc_clock_ns: " << adc_clock_ns << NL;

  delay_ms( 100 );

  uint32_t stime_ns = ADC_conv_time_tick( stime_idx, n_ch, BOARD_ADC_DEFAULT_BITS );
  if( stime_ns == 0xFFFFFFFF ) {
    std_out << "# error: fail to calculate conversion time" NL;
    return 8;
  }
  stime_ns *= adc_clock_ns;

  uint32_t t_wait0 = 1 + uint32_t( ( 999 + stime_ns ) / 1000 ); // in ms, unused here

  if( t_step_us * 1000 <= stime_ns ) {
    std_out << "# warn: time step (" << t_step_us * 1000 << ") ns < conversion time (" << stime_ns << ") ns" NL;
  }

  std_out << "# stime_idx= " << stime_idx << " 10ticks= " << adc_arch_sampletimes[stime_idx].stime10
          << " stime_ns= "  << stime_ns  << " code= " <<  adc_arch_sampletimes[stime_idx].code
          << " t_wait0= " << t_wait0 << " ms" NL;

  delay_ms( 100 );

  adc.prepare_single_manual( div_bits, adc_arch_sampletimes[stime_idx].code, BOARD_ADC_DEFAULT_RESOLUTION );

  if( UVAR('d') > 0 ) {
    adc.pr_state();
  }
  if( UVAR('d') > 1 ) {
    dump32( BOARD_ADC_DEFAULT_DEV, 0x200 );
  }

  std_out << "Prepared!" << NL;
  delay_ms( 100 );

  if( ! adc.init_common() ) {
    std_out << "# error: fail to init ADC: errno= " << errno << NL;
  }

  if( UVAR('d') > 0 ) {
    adc.pr_state();
  }
  if( UVAR('d') > 1 ) {
    dump32( BOARD_ADC_DEFAULT_DEV, 0x100 );
  }
  // log_reset();

  // or such
  ADC_freq_info fi;
  ADC_calcfreq( &adc.hadc, &fi );
  std_out << "# ADC: freq_in: " << fi.freq_in << " freq: " << fi.freq
          << " div: " << fi.div << " div1: " << fi.div1 << " div2: " << fi.div2
          << " bits: " << HexInt( fi.devbits ) << NL;

  StatIntData sdat( 1, k_all );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( decltype(+n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();

    if( !adc.start() )  {
      std_out << "# error:  !! ADC Start error" NL;
      break;
    }

    int v = 0;
    if( adc.poll_and_read( v ) ) {
      xfloat vv = k_all * v;
      sdat.add( &v );

      if( UVAR('b') == 0 ) {
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


  if( UVAR('d') > 0 ) {
    adc.pr_state();
  }
  if( UVAR('d') > 1 ) {
    dump32( BOARD_ADC_DEFAULT_DEV, 0x200 );
  }


  // HAL_ADC_Stop( &adc.hadc1 );

  return 0;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != BOARD_ADC_DEFAULT_DEV ) {
    return;
  }
  BOARD_ADC_DEFAULT_EN;

  adc.init_gpio_channels();
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != BOARD_ADC_DEFAULT_DEV ) {
    return;
  }

  BOARD_ADC_DEFAULT_DIS;
}






// TODO: if ! HAVE_FLOAT
//int vv = v * ( UVAR('v') / 100 ) / BOARD_ADC_DEFAULT_MAX; // 100 = 1000/10
//std_out << " v= " << v <<  " vv= " << FloatMult( vv, 4 );


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

