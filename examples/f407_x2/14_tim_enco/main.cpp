#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

UsbcdcIO usbcdc;

TIM_HandleTypeDef tim1h;
TIM_Encoder_InitTypeDef enco_cfg;
void init_enco();

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

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
}

STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 0;
  }

  delay_bad_ms( 200 );  leds.write( 1 );

  UVAR('t') = 10;
  UVAR('n') = 2000;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_USBCDC_AS_STDIO(usbcdc);

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
  break_flag = 0;


  int old_cnt = -1;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint16_t t_cnt = tim1h.Instance->CNT;
    // uint16_t t_ccr1 = tim1h.Instance->CCR1;
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
  break_flag = 0;

  pr( NL "test0 end." NL );
  return 0;
}

//  ----------------------------- configs ----------------
//
void init_enco()
{
  tim1h.Instance = TIM1;
  tim1h.Init.Period            = 0xFFFF;
  tim1h.Init.Prescaler         = 0; // 0?
  tim1h.Init.ClockDivision     = 0;
  tim1h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim1h.Init.RepetitionCounter = 0;

  enco_cfg.EncoderMode    = TIM_ENCODERMODE_TI12;
  enco_cfg.IC1Polarity    = TIM_ICPOLARITY_RISING;
  enco_cfg.IC1Selection   = TIM_ICSELECTION_DIRECTTI;
  enco_cfg.IC1Prescaler   = TIM_ICPSC_DIV1;
  enco_cfg.IC1Filter      = 0x0F; /// 0-F
  enco_cfg.IC2Polarity    = TIM_ICPOLARITY_RISING;
  enco_cfg.IC2Selection   = TIM_ICSELECTION_DIRECTTI;
  enco_cfg.IC2Prescaler   = TIM_ICPSC_DIV1;
  enco_cfg.IC2Filter      = 0x0F; /// 0-F

  if( HAL_TIM_Encoder_Init( &tim1h, &enco_cfg ) != HAL_OK ) {
    UVAR('e') = 1;
    return;
  }
  HAL_TIM_Encoder_Start( &tim1h, TIM_CHANNEL_ALL );
}




FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

