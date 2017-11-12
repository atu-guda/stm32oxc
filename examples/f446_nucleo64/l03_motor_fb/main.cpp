#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

PinsOut ctlEn( GPIOA, 8, 1 );
PinsOut ctlAB( GPIOA, 9, 2 );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};



int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') = 1000;
  UVAR('g') = 10;
  UVAR('n') = 1;

  ctlEn.initHW();  ctlEn.write( 0 );
  ctlAB.initHW();  ctlAB.write( 0 );

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS_UART;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int tg     = arg2long_d( 1, argc, argv, UVAR('g'), 0 );
  int dir    = arg2long_d( 2, argc, argv, 0, 0, 1 );
  int n_step = arg2long_d( 3, argc, argv, UVAR('n'), 1, 100 );
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: tg= " ); pr_d( tg ); pr( " t= " ); pr_d( t_step ); pr( " n_step= " ); pr_d( n_step );
  pr( NL );

  leds.reset( 1 ); leds.reset( 6 );
  ctlEn.write( 0 );
  ctlAB.write( dir ? 1 : 2 ); leds.set( dir ? 2 : 4 );

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;

  break_flag = 0;
  for( int i=0; i<n_step && !break_flag; ++i ) {
    TickType_t tcc = xTaskGetTickCount();
    pr( " step i= " ); pr_d( i );
    pr( "  tick: "); pr_d( tcc - tc00 );
    pr( NL );
    leds.set( 1 ); ctlEn.set( 1 );
    delay_ms( tg );
    leds.reset( 1 ); ctlEn.reset( 1 );
    vTaskDelayUntil( &tc0, t_step );
  }

  ctlEn.write( 0 );  ctlAB.write( 0 );
  leds.reset( 1 ); leds.reset( 6 );

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

