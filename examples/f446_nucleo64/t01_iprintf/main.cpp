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
  // dev_console.puts( "0123456789---main()---ABCDEF" NL );

  UVAR('t') = 1000;
  UVAR('n') = 10;

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
  iprintf( NL "Test0: n= %u t= %lu", n, t_step );

  iprintf( "iprintf test: '%c' %d %ld %u " NL, 'z', 123, 1000000000L, -1 );
  // ecvt( 3.1415926, 5, 0, 0 ); // costs like full printf

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    // action
    uint32_t tmc = HAL_GetTick();
    iprintf( " Fake Action i= %d  tick: %lu " NL, i, tmc - tm00 );

    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

