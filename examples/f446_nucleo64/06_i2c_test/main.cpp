#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
} // extern "C"

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 ); // zero add means no real device


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;

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
  int st_a = arg2long_d( 1, argc, argv,   2,    0, 127 );
  int en_a = arg2long_d( 2, argc, argv, 127, st_a, 127 );
  pr( NL "Test0: st_a= " ); pr_h( st_a ); pr( " en_a= " ); pr_h( en_a );
  pr( NL );
  i2cd.resetDev();

  // uint8_t val;
  int i_err;
  for( uint8_t ad = (uint16_t)st_a; ad <= (uint16_t)en_a && ! break_flag; ++ad ) {
    bool rc = i2cd.ping( ad );
    i_err = i2cd.getErr();
    delay_ms( 10 );
    if( !rc ) {
      continue;
    }
    pr( "ad = " ); pr_h( ad ); pr( "  rc= " ); pr_d( rc ) ; pr( "  err= " ); pr_d( i_err ); pr(NL);
  }

  return 0;
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

