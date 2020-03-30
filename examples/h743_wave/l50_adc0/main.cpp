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
 " var v - reference voltage in uV " NL;

int adc_arch_init_exa_1ch_manual( uint32_t presc, uint32_t sampl_cycl );
ADC_HandleTypeDef hadc1;
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

  UVAR('t') = 100;
  UVAR('n') = 20;
  UVAR('v') = v_adc_ref;

  UVAR('e') =  adc_arch_init_exa_1ch_manual( BOARD_ADC_DEFAULT_CLOCK, BOARD_ADC_DEFAULT_SAMPL_LARGE );

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}



// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  NL "Test0: n= " << n << " t= " << t_step << NL;
  int v = 0;
  xfloat k_all = 1e-6f * UVAR('v') / BOARD_ADC_DEFAULT_MAX;

  StatIntData sdat( 1, k_all );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();

    if( HAL_ADC_Start( &hadc1 ) != HAL_OK )  {
      std_out << "# error:  !! ADC Start error" NL;
      break;
    }

    HAL_ADC_PollForConversion( &hadc1, 10 );
    // std_out << "# ADC1.ISR= " << HexInt( ADC1->ISR );

    v = 0;
    if( HAL_IS_BIT_SET( HAL_ADC_GetState( &hadc1 ), HAL_ADC_STATE_REG_EOC ) )  {
      v = HAL_ADC_GetValue( &hadc1 );
      sdat.add( &v );

      // TODO: if ! HAVE_FLOAT
      //int vv = v * ( UVAR('v') / 100 ) / BOARD_ADC_DEFAULT_MAX; // 100 = 1000/10
      //std_out << " v= " << v <<  " vv= " << FloatMult( vv, 4 );

      xfloat vv = k_all * v;
      if( UVAR('x') == 0 ) {
        std_out << FmtInt( i, 5 ) << ' ' << FmtInt( tcc - tm00, 8 )
                << ' ' << FmtInt( v, 5 ) << ' ' << XFmt( vv, cvtff_auto, 9, 6 ) << NL;
      }
    } else {
      std_out << "# warn: REG_EOC not set!" NL;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  sdat.calc();
  std_out << sdat << NL;

  if( UVAR('d') > 1 ) {
    dump32( ADC1, 0x200 );
  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

