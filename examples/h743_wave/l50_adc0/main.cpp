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
 " var t - delay time in ms" NL
 " var n - default number of measurements" NL
 " var s - sample time index" NL
 " var x - supress normal output" NL
 " var v - reference voltage in uV " NL;

int adc_arch_init_exa_1ch_manual( uint32_t presc, uint32_t sampl_cycl );
ADC_HandleTypeDef hadc1;
int v_adc_ref = BOARD_ADC_COEFF; // in uV, measured before test

// TODO: to common header
struct AdcSampleTimeInfo {
  uint32_t code;
  uint32_t stime10; //* in 10 * ADC_Cycles
};

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

  UVAR('t') = 100;
  UVAR('n') = 20;
  UVAR('s') = adc_arch_sampletimes_n;
  UVAR('v') = v_adc_ref;


  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}

constexpr uint32_t adc_arch_div2divcode( uint32_t div )
{
  switch( div ) {
    case   1: return ADC_CLOCK_ASYNC_DIV1;
    case   2: return ADC_CLOCK_ASYNC_DIV2;
    case   4: return ADC_CLOCK_ASYNC_DIV4;
    case   6: return ADC_CLOCK_ASYNC_DIV6;
    case   8: return ADC_CLOCK_ASYNC_DIV8;
    case  10: return ADC_CLOCK_ASYNC_DIV10;
    case  12: return ADC_CLOCK_ASYNC_DIV12;
    case  16: return ADC_CLOCK_ASYNC_DIV16;
    case  32: return ADC_CLOCK_ASYNC_DIV32;
    case  64: return ADC_CLOCK_ASYNC_DIV64;
    case 128: return ADC_CLOCK_ASYNC_DIV128;
    case 256: return ADC_CLOCK_ASYNC_DIV256;
    default:  return 0xFFFFFFFF; // ???
  }
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  const int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  const uint32_t t_step = UVAR('t');
  const xfloat k_all = 1e-6f * UVAR('v') / BOARD_ADC_DEFAULT_MAX;

  // TODO: to clock config file
  const uint32_t adc_arch_clock_in = 96000000;
  const uint32_t adc_arch_clock_div = 4;
  const uint32_t adc_arch_clock    = adc_arch_clock_in / adc_arch_clock_div;
  constexpr uint32_t adc_arch_clock_divcode = adc_arch_div2divcode( adc_arch_clock_div );
  // ???? * 2 for Rev 'V' devices???
  const uint32_t adc_arch_clock_ns = (unsigned)( 1 + 1000000000LL / adc_arch_clock );

  // std_out << "## div= " << HexInt(adc_arch_clock_divcode) << " div4= " << HexInt(ADC_CLOCK_ASYNC_DIV4) << NL;

  unsigned stime_idx = ( (unsigned)UVAR('s') < adc_arch_sampletimes_n ) ? UVAR('s') : (adc_arch_sampletimes_n - 1);
  unsigned stime_ns = (unsigned)( ( 9 + adc_arch_clock_ns * adc_arch_sampletimes[stime_idx].stime10  ) / 10 );

  std_out <<  NL "Test0: n= " << n << " t= " << t_step
          << " stime_idx= " << stime_idx << " 10ticks= " << adc_arch_sampletimes[stime_idx].stime10
          << " stime_ns= "  << stime_ns  << " code= " <<  adc_arch_sampletimes[stime_idx].code << NL;

  if( ! adc_arch_init_exa_1ch_manual( adc_arch_clock_divcode, adc_arch_sampletimes[stime_idx].code ) ) {
    std_out << "# error: fail to init ADC: errno= " << errno << NL;
  }

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

  HAL_ADC_Stop( &hadc1 );

  return 0;
}

// TODO: if ! HAVE_FLOAT
//int vv = v * ( UVAR('v') / 100 ) / BOARD_ADC_DEFAULT_MAX; // 100 = 1000/10
//std_out << " v= " << v <<  " vv= " << FloatMult( vv, 4 );

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

