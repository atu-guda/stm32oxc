#include <cstring>
#include <cstdlib>
#include <stdio.h>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

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

  // HAL_UART_Transmit( &uah, (uint8_t*)"START\r\n", 7, 100 );
  // dev_console.sendStrSync( "0123456789---main()---ABCDEF" NL );

  UVAR('t') = 1000;
  UVAR('n') = 10;

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  dev_console.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  iprintf( NL "Test0: n= %u t= %lu", n, t_step );

  int prty = uxTaskPriorityGet( 0 );
  pr_sdx( prty );
  const char *nm = pcTaskGetName( 0 );
  iprintf( "name: \"%s\"" NL, nm );

  iprintf( "iprintf test: '%c' %d %ld %u " NL, 'z', 123, 1000000000L, -1 );
  // ecvt( 3.1415926, 5, 0, 0 ); // costs like full printf

  // log_add( "Test0 " );
  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    TickType_t tcc = xTaskGetTickCount();
    uint32_t tmc = HAL_GetTick();
    iprintf( " Fake Action i= %d  tick: %lu   ms_tick: %lu" NL, i, tcc - tc00, tmc - tm0 );
    delay_ms_until_brk( &tc0, t_step );
  }

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

