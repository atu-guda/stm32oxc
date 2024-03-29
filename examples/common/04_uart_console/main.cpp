#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_test_rate( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST_RATE { "test_rate", 0, cmd_test_rate, "[ n [len] ] - test output rate"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TEST_RATE,
  nullptr
};




int main(void)
{
  STD_PROLOG_UART;

  // HAL_UART_Transmit( &uah, (uint8_t*)"START\r\n", 7, 100 );
  // usartio.puts_s( "0123456789---main()---ABCDEF" NL );

  UVAR('t') = 100;
  UVAR('n') = 10;

  BOARD_POST_INIT_BLINK;

  BOARD_CREATE_STD_TASKS;

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  dev_console.puts( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}





// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

