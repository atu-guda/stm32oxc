#include <climits>
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
int cmd_test_val( int argc, const char * const * argv );
const CmdInfo CMDINFO_TEST_VAL { "test_val", 'v', cmd_test_val, "[arg ] - test arg2long* converter"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST_DELAYS,
  &CMDINFO_TEST_RATE,
  &CMDINFO_TEST_VAL,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') =  20;
  UVAR('u') = -23;
  UVAR('v') =  42;

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

int cmd_test_val( int argc, const char * const * argv )
{
  int v0 = arg2long_d( 1, argc, argv,  UVAR('v'), INT_MIN, INT_MAX );
  int v1 = arg2long_d( 2, argc, argv,  UVAR('u'),    -500,    1500 );
  std_out << "# " << v0 << ' ' << v1 << NL;
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

