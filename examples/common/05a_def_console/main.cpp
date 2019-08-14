#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;


BOARD_CONSOLE_DEFINES;

#if defined(USE_OXC_CONSOLE_UART)
  const char* common_help_string = "Appication to test common console with auto I/O selection: UART" NL;
#elif defined(USE_OXC_CONSOLE_USB_CDC)
  const char* common_help_string = "Appication to test common console with auto I/O selection: USB_CDC" NL;
#else
  const char* common_help_string = "Appication to test common console with auto I/O selection: Unknown" NL;
#endif

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
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

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





// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

