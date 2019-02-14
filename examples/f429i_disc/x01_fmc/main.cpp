#include <cstring>

#include <oxc_auto.h>

#include <board_sdram.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

SDRAM_HandleTypeDef hsdram;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test SDRAM usage via FMC" NL;
// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

void idle_main_task()
{
  leds.toggle( 1 );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  leds.write( 0x03 );  delay_bad_ms( 200 );
  bsp_init_sdram( &hsdram );

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  pr( NL "Test0: n= " ); pr_d( n );
  pr( NL );
  delay_ms( 100 );
  for( int i=0; i<n; ++i ) {
    SDRAM_ADDR[i] = (uint8_t)(i+0x20);
  }

  dump8( SDRAM_ADDR, n, true );

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

