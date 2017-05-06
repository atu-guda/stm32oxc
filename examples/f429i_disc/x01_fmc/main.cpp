#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

#include <bsp/board_stm32f429discovery_sdram.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

SDRAM_HandleTypeDef hsdram;

BOARD_CONSOLE_DEFINES;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
} // extern "C"



int main(void)
{
  BOARD_PROLOG;

  bsp_init_sdram( &hsdram );
  leds.write( 0x03 );  delay_bad_ms( 200 );


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



//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

