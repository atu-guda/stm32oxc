#include <climits>
#include <oxc_auto.h>
#include <oxc_main.h>

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
DCL_CMD_REG( test_val, 'v', "[arg ] - test arg2long* converter"  );
DCL_CMD_REG( test_mdelay, 'X', "[v] - test main loop delay"  );
DCL_CMD_REG( delay_calibrate, 'D', " - calibrate dumb delay"  );


void idle_main_task()
{
  leds.toggle( 1 );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 100;
  UVAR_n =  20;
  UVAR_u = -23;
  UVAR_v =  42;

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

// used
// void test_delays_misc( int n, uint32_t t_step, int tp );
// void test_output_rate( int n, int sl, int do_flush );
// from ../../../src/oxc_debug1.cpp

int cmd_test_val( int argc, const char * const * argv )
{
  int v0 = arg2long_d( 1, argc, argv,  UVAR_v, INT_MIN, INT_MAX );
  int v1 = arg2long_d( 2, argc, argv,  UVAR_u,    -500,    1500 );
  std_out << "# " << v0 << ' ' << v1 << NL;
  return 0;
}

int cmd_test_mdelay( int argc, const char * const * argv )
{
  int v = arg2long_d( 1, argc, argv,  10, 0, 5000 );
  main_loop_delay_nortos = v;
  std_out << "# mdelay: " << main_loop_delay_nortos << NL;
  return 0;
}

int cmd_delay_calibrate( int argc, const char * const * argv )
{
  std_out << "# old delay_calibrate_value:" << delay_calibrate_value << NL;
  do_delay_calibrate();
  std_out << "# new delay_calibrate_value:" << delay_calibrate_value << NL;
  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

