#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test USB console w/o FreeRTOS" NL;

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
  leds.toggle( 1 );
}



int main(void)
{
  // STD_PROLOG_USBCDC;

  // STD_PROLOG_START;
  HAL_Init();
  leds.initHW();
  leds.write( BOARD_LEDS_ALL );
  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 1;
  }
  ADD_DEVIO_TICKFUN;

  delay_ms( PROLOG_LED_TIME ); leds.write( 0x01 ); delay_ms( PROLOG_LED_TIME ); leds.write( 0x02 );
  if( ! dev_console.init() ) {
    die4led( 1 );
  }
  // HAL_Delay( PROLOG_LED_TIME );
  leds.write( 0x04 ); // delay_ms( PROLOG_LED_TIME );

  global_smallrl = &srl;
  SET_USBCDC_AS_STDIO( dev_console );
  std_out.setOut( devio_fds[1] );

  leds.write( 0x08 );

  UVAR('t') = 100;
  UVAR('n') =  20;

  // BOARD_POST_INIT_BLINK;
  leds.write( 0x08 );

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

