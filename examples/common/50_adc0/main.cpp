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
 " var v - reference voltage in mV " NL;

int adc_init_exa_1ch_manual( uint32_t presc, uint32_t sampl_cycl );
ADC_HandleTypeDef hadc1;
int v_adc_ref = 3250; // in mV, measured before test


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

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  default_main_loop();
  vTaskDelete(NULL);
}




// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  STDOUT_os;
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  os <<  NL "Test0: n= " << n << " t= " << t_step << NL; os.flush();
  uint16_t v = 0;

  // log_add( "Test0 " );
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;

  for( int i=0; i<n && !break_flag; ++i ) {
    TickType_t tcc = xTaskGetTickCount();
    os << "ADC start  i= " << i << "  tick: " << ( tcc - tc00 );
    if( HAL_ADC_Start( &hadc1 ) != HAL_OK )  {
      os << "  !! ADC Start error" NL; os.flush();
      break;
    }
    HAL_ADC_PollForConversion( &hadc1, 10 );
    os << " ADC1.SR= " << HexInt( ADC1->SR );
    v = 0;
    if( HAL_IS_BIT_SET( HAL_ADC_GetState( &hadc1 ), HAL_ADC_STATE_REG_EOC ) )  {
      v = HAL_ADC_GetValue( &hadc1 );
      int vv = v * 10 * UVAR('v') / 4096;
      os << " v= " << v <<  " vv= " << FloatMult( vv, 4 );
    }

    os << NL; os.flush();
    delay_ms_until_brk( &tc0, t_step );
  }

  pr( NL );

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

