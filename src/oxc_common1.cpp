#include <oxc_gpio.h>
#include <oxc_smallrl.h>
#include <oxc_common1.h>

#if !defined USE_FREERTOS
#error This file in only for FreeRTOS enabled projects.
#endif

using namespace SMLRL;

volatile int task_leds_step = 5;

void task_leds( void *prm UNUSED_ARG )
{
  while (1) {
    int dly = task_leds_step * TASK_LEDS_QUANT;
    if( dly < 10 )    { dly = 10;    };
    if( dly > 10000 ) { dly = 10000; };
    #ifdef LED_BSP_IDLE
      leds.toggle( LED_BSP_IDLE );
    #else
      #waring LED_BSP_IDLE is not defined
    #endif
    delay_ms( dly );
  }
  vTaskDelete(NULL);
}

#ifndef MAIN_LOOP_WAIT
#define MAIN_LOOP_WAIT 60000
#endif

void default_main_loop()
{
  uint32_t nl = 0;
  delay_ms( 10 );
  pr( "*=*** Main loop: ****** " NL );
  delay_ms( 20 );
  if( ! global_smallrl ) {
    die( 0x01 );
  }

  global_smallrl->setSigFun( smallrl_sigint );
  global_smallrl->set_ps1( "\033[32m#\033[0m ", 2 );
  global_smallrl->re_ps();
  global_smallrl->set_print_cmd( true );


  idle_flag = 1;
  while(1) {
    ++nl;
    if( idle_flag == 0 ) {
      pr_sd( ".. main idle  ", nl );
      global_smallrl->redraw();
    }
    idle_flag = 0;
    delay_ms( MAIN_LOOP_WAIT );
  }
}

void task_gchar( void *prm UNUSED_ARG )
{
  char sc[2] = { 0, 0 };
  while (1) {
    int n = recvByte( 0, sc, 10000 );
    if( n ) {
      if( global_smallrl ) {
        global_smallrl->addChar( sc[0] );
      }
      idle_flag = 1;
    }
  }
  vTaskDelete(NULL);
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

void sigint( int v UNUSED_ARG )
{
  smallrl_sigint();
}

void smallrl_sigint(void)
{
  break_flag = 1;
  idle_flag = 1;
  leds.toggle( LED_BSP_ERR );
}

