#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test ADC in one-shot mode one channel" NL
 " var t - delay time in ms" NL
 " var n - default number of measurements" NL
 " var v - reference voltage in uV " NL;

int adc_init_exa_1ch_manual( uint32_t presc, uint32_t sampl_cycl );
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

  UVAR('t') = 1000;
  UVAR('n') = 20;
  UVAR('v') = v_adc_ref;

  UVAR('e') =  adc_init_exa_1ch_manual( ADC_CLOCK_SYNC_PCLK_DIV4, ADC_SAMPLETIME_144CYCLES );

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
  std_out <<  NL "Test0: n= " << n << " t= " << t_step << NL; std_out.flush();
  uint16_t v = 0;

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    std_out << "i= " << i << "  tick: " << ( tcc - tm00 );
    if( HAL_ADC_Start( &hadc1 ) != HAL_OK )  {
      std_out << "  !! ADC Start error" NL; std_out.flush();
      break;
    }
    HAL_ADC_PollForConversion( &hadc1, 10 );
    std_out << " ADC1.SR= " << HexInt( ADC1->SR );
    v = 0;
    if( HAL_IS_BIT_SET( HAL_ADC_GetState( &hadc1 ), HAL_ADC_STATE_REG_EOC ) )  {
      v = HAL_ADC_GetValue( &hadc1 );
      int vv = v * ( UVAR('v') / 100 ) / 4096; // 100 = 1000/10
      std_out << " v= " << v <<  " vv= " << FloatMult( vv, 4 );
    }

    std_out << NL; std_out.flush();
    delay_ms_until_brk( &tm0, t_step );
  }

  pr( NL );

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

