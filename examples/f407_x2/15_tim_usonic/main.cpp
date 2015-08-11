#include <cstring>
#include <cstdlib>

#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;

UsbcdcIO usbcdc;

TIM_HandleTypeDef tim1h;
TIM_OC_InitTypeDef  tim_oc_cfg;
TIM_IC_InitTypeDef  tim_ic_cfg;
// TIM_SlaveConfigTypeDef tim_slave_cfg;
void init_usonic();

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

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  leds.write( 0x00 );

  UVAR('t') = 1000;
  UVAR('n') = 20;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  usbcdc.init();
  usbcdc.setOnSigInt( sigint );
  devio_fds[0] = &usbcdc; // stdin
  devio_fds[1] = &usbcdc; // stdout
  devio_fds[2] = &usbcdc; // stderr
  delay_ms( 50 );

  init_usonic();

  delay_ms( 10 );
  pr( "*=*** Main loop: ****** " NL );
  delay_ms( 20 );

  srl.setSigFun( smallrl_sigint );
  srl.set_ps1( "\033[32m#\033[0m ", 2 );
  srl.re_ps();
  srl.set_print_cmd( true );


  idle_flag = 1;
  while(1) {
    ++nl;
    if( idle_flag == 0 ) {
      pr_sd( ".. main idle  ", nl );
      srl.redraw();
    }
    idle_flag = 0;
    delay_ms( 60000 );
    // delay_ms( 1 );

  }
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


  for( int i=0; i<n && !break_flag; ++i ) {
    pr( "[" ); pr_d( i );
    pr( "]  l= " ); pr_d( UVAR('l') );
    pr( NL );
    vTaskDelayUntil( &tc0, t_step );
    // delay_ms( t_step );
  }

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

//  ----------------------------- configs ----------------
//
void init_usonic()
{
  tim1h.Instance = TIM1;
  tim1h.Init.Prescaler         = 997; // 5.8 mks approx 1mm
  tim1h.Init.Period            = 32000; // 8500;
  tim1h.Init.ClockDivision     = 0;
  tim1h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim1h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim1h ) != HAL_OK ) {
    UVAR('e') = 1; // like error
    return;
  }

  tim_oc_cfg.OCMode = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode = TIM_OCFAST_DISABLE;
  tim_oc_cfg.Pulse = 3; // 3 = appex 16 mks
  if( HAL_TIM_PWM_ConfigChannel( &tim1h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 11;
    return;
  }
  if( HAL_TIM_PWM_Start( &tim1h, TIM_CHANNEL_1 ) != HAL_OK ) {
    UVAR('e') = 12;
    return;
  }

  // tim_ic_cfg.ICPolarity = TIM_ICPOLARITY_RISING;
  tim_ic_cfg.ICPolarity = TIM_ICPOLARITY_BOTHEDGE;
  tim_ic_cfg.ICSelection = TIM_ICSELECTION_DIRECTTI;
  tim_ic_cfg.ICPrescaler = TIM_ICPSC_DIV1;
  tim_ic_cfg.ICFilter = 0; // 0 - 0x0F

  if( HAL_TIM_IC_ConfigChannel( &tim1h, &tim_ic_cfg, TIM_CHANNEL_2 ) != HAL_OK )
  {
    UVAR('e') = 21;
    return;
  }

  // tim_slave_cfg.SlaveMode = TIM_SLAVEMODE_RESET;
  // tim_slave_cfg.InputTrigger  = TIM_TS_TI2FP2;
  // if( HAL_TIM_SlaveConfigSynchronization( &tim1h, &tim_slave_cfg ) != HAL_OK ) {
  //   UVAR('e') = 22;
  //   return;
  // }

  HAL_NVIC_SetPriority( TIM1_CC_IRQn, 5, 0 );
  HAL_NVIC_EnableIRQ( TIM1_CC_IRQn );

  if( HAL_TIM_IC_Start_IT( &tim1h, TIM_CHANNEL_2 ) != HAL_OK ) {
    UVAR('e') = 23;
  }
}

void TIM1_CC_IRQHandler(void)
{
  HAL_TIM_IRQHandler( &tim1h );
}

void HAL_TIM_IC_CaptureCallback( TIM_HandleTypeDef *htim )
{
  uint32_t cap2;
  static uint32_t c_old = 0xFFFFFFFF;
  if( htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2 )  {
    cap2 = HAL_TIM_ReadCapturedValue( htim, TIM_CHANNEL_2 );
    if( cap2 > c_old ) {
      UVAR('l') = cap2 - c_old ;
    }
    c_old = cap2;

    UVAR('m') = cap2;
    UVAR('z') = htim->Instance->CNT;
  }
}



FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

