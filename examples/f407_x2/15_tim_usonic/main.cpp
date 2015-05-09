#include <cstring>
#include <cstdlib>


#include <bsp/board_stm32f407_atu_x2.h>
#include <oxc_gpio.h>
#include <oxc_usbcdcio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_smallrl.h>

#include "usbd_desc.h"
#include <usbd_cdc.h>
#include <usbd_cdc_interface.h>

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

// SmallRL storage and config
int smallrl_print( const char *s, int l );
int smallrl_exec( const char *s, int l );
void smallrl_sigint(void);


SmallRL srl( smallrl_print, smallrl_exec );
// SmallRL srl( smallrl_print, exec_queue );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int idle_flag = 0;
int break_flag = 0;


const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {

void task_main( void *prm UNUSED_ARG );
void task_leds( void *prm UNUSED_ARG );
void task_gchar( void *prm UNUSED_ARG );

void TIM1_CC_IRQHandler(void);

}

STD_USBCDC_SEND_TASK( usbcdc );

int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  leds.write( 0x00 );

  global_smallrl = &srl;

  user_vars['t'-'a'] = 1000;
  user_vars['n'-'a'] = 20;


  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usbcdc_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   2, nullptr ); // 1?

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_leds( void *prm UNUSED_ARG )
{
  while (1)
  {
    leds.toggle( BIT1 );
    delay_ms( 500 );
  }
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  usbcdc.init();
  delay_ms( 50 );

  init_usonic();

  delay_ms( 10 );
  pr( "*=*** Main loop: ****** " NL );

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

void task_gchar( void *prm UNUSED_ARG )
{
  char sc[2] = { 0, 0 };
  while (1) {
    int n = usbcdc.recvByte( sc, 10000 );
    if( n ) {
      srl.addChar( sc[0] );
      idle_flag = 1;
    }
  }
  vTaskDelete(NULL);
}


void _exit( int rc )
{
  exit_rc = rc;
  die4led( rc );
}


int pr( const char *s )
{
  if( !s || !*s ) {
    return 0;
  }
  prl( s, strlen(s) );
  return 0;
}

int prl( const char *s, int l )
{
  // usbcdc.sendBlockSync( s, l );
  usbcdc.sendBlock( s, l );
  idle_flag = 1;
  return 0;
}

// ---------------------------- smallrl -----------------------


int smallrl_print( const char *s, int l )
{
  prl( s, l );
  return 1;
}

int smallrl_exec( const char *s, int l )
{
  exec_direct( s, l );
  return 1;
}


void smallrl_sigint(void)
{
  break_flag = 1;
  idle_flag = 1;
  leds.toggle( BIT3 );
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = user_vars['n'-'a'];
  int t = user_vars['t'-'a'];
  pr( NL "Test0: n= " ); pr_d( n ); pr( "  t= " ); pr_d( t ); pr( NL );

  delay_ms( 10 );
  break_flag = 0;


  for( int i=0; i<n && !break_flag; ++i ) {
    pr( "[" ); pr_d( i );
    pr( "]  l= " ); pr_d( user_vars['l'-'a'] );
    pr( NL );
    delay_ms( t );
  }

  pr( NL "test0 end." NL );
  return 0;
}

//  ----------------------------- configs ----------------
//
void init_usonic()
{
  tim1h.Instance = TIM1;
  tim1h.Init.Prescaler         = 997; // 5.8 mks approx 1mm
  tim1h.Init.Period            = 8500;
  tim1h.Init.ClockDivision     = 0;
  tim1h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  tim1h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &tim1h ) != HAL_OK ) {
    user_vars['e'-'a'] = 1; // like error
    return;
  }

  tim_oc_cfg.OCMode = TIM_OCMODE_PWM1;
  tim_oc_cfg.OCPolarity = TIM_OCPOLARITY_HIGH;
  tim_oc_cfg.OCFastMode = TIM_OCFAST_DISABLE;
  tim_oc_cfg.Pulse = 3; // 3 = appex 16 mks
  if( HAL_TIM_PWM_ConfigChannel( &tim1h, &tim_oc_cfg, TIM_CHANNEL_1 ) != HAL_OK ) {
    user_vars['e'-'a'] = 11;
    return;
  }
  if( HAL_TIM_PWM_Start( &tim1h, TIM_CHANNEL_1 ) != HAL_OK ) {
    user_vars['e'-'a'] = 12;
    return;
  }

  // tim_ic_cfg.ICPolarity = TIM_ICPOLARITY_RISING;
  tim_ic_cfg.ICPolarity = TIM_ICPOLARITY_BOTHEDGE;
  tim_ic_cfg.ICSelection = TIM_ICSELECTION_DIRECTTI;
  tim_ic_cfg.ICPrescaler = TIM_ICPSC_DIV1;
  tim_ic_cfg.ICFilter = 0; // 0 - 0x0F

  if( HAL_TIM_IC_ConfigChannel( &tim1h, &tim_ic_cfg, TIM_CHANNEL_2 ) != HAL_OK )
  {
    user_vars['e'-'a'] = 21;
    return;
  }

  // tim_slave_cfg.SlaveMode = TIM_SLAVEMODE_RESET;
  // tim_slave_cfg.InputTrigger  = TIM_TS_TI2FP2;
  // if( HAL_TIM_SlaveConfigSynchronization( &tim1h, &tim_slave_cfg ) != HAL_OK ) {
  //   user_vars['e'-'a'] = 22;
  //   return;
  // }

  HAL_NVIC_SetPriority( TIM1_CC_IRQn, 5, 0 );
  HAL_NVIC_EnableIRQ( TIM1_CC_IRQn );

  if( HAL_TIM_IC_Start_IT( &tim1h, TIM_CHANNEL_2 ) != HAL_OK ) {
    user_vars['e'-'a'] = 23;
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
      user_vars['l'-'a'] = cap2 - c_old ;
    }
    c_old = cap2;

    user_vars['m'-'a'] = cap2;
    user_vars['z'-'a'] = htim->Instance->CNT;
  }
}



FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

