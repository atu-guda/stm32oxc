#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

const char* common_help_string = "Appication to test common console via UART" NL;

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

void idle_main_task()
{
  // leds.toggle( 1 );
}



int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') = 100;
  UVAR('n') =  20;

  BOARD_POST_INIT_BLINK;


  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

