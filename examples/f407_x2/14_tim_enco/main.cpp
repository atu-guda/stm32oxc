#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

TIM_HandleTypeDef tim_h;
void init_enco();

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
} // extern "C"

int main(void)
{
  BOARD_PROLOG;


  UVAR('t') = 10;
  UVAR('n') = 2000;


  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  init_enco();

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( NL );

  TickType_t tc0 = xTaskGetTickCount();

  delay_ms( 10 );

  int old_cnt = -1;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint16_t t_cnt = tim_h.Instance->CNT;
    // uint16_t t_ccr1 = tim_h.Instance->CCR1;
    if( t_cnt != old_cnt ) {
      pr( "[" ); pr_d( i );
      pr( "]  CNT= " ); pr_d( t_cnt );
      pr( "  D= " ); pr_d( t_cnt - old_cnt );
      // pr( "  CCR1= " ); pr_d( t_ccr1 );
      pr( NL );
    }
    vTaskDelayUntil( &tc0, t_step );
    old_cnt = t_cnt;
    // delay_ms( t_step );
  }

  return 0;
}

//  ----------------------------- configs ----------------

void init_enco()
{
  tim_h.Instance               = TIM_EXA;
  tim_h.Init.Period            = 0xFFFF;
  tim_h.Init.Prescaler         = 0; // 0?
  tim_h.Init.ClockDivision     = 0;
  tim_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim_h.Init.RepetitionCounter = 0;

  TIM_Encoder_InitTypeDef enco_cfg;
  enco_cfg.EncoderMode    = TIM_ENCODERMODE_TI12;
  enco_cfg.IC1Polarity    = TIM_ICPOLARITY_RISING;
  enco_cfg.IC1Selection   = TIM_ICSELECTION_DIRECTTI;
  enco_cfg.IC1Prescaler   = TIM_ICPSC_DIV1;
  enco_cfg.IC1Filter      = 0x0F; /// 0-F
  enco_cfg.IC2Polarity    = TIM_ICPOLARITY_RISING;
  enco_cfg.IC2Selection   = TIM_ICSELECTION_DIRECTTI;
  enco_cfg.IC2Prescaler   = TIM_ICPSC_DIV1;
  enco_cfg.IC2Filter      = 0x0F; /// 0-F

  if( HAL_TIM_Encoder_Init( &tim_h, &enco_cfg ) != HAL_OK ) {
    UVAR('e') = 1;
    return;
  }
  HAL_TIM_Encoder_Start( &tim_h, TIM_CHANNEL_ALL );
}





// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

