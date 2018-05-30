#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const int go_tick = 100; // 0.1 s

PinsOut motor_dir( GPIOC, 5, 5 );
const int motor_bits_r = 0x03; // bit 0x04 is reserved
const int motor_bits_l = 0x18;
const int motor_bits   = motor_bits_r | motor_bits_l;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_go( int argc, const char * const * argv );
CmdInfo CMDINFO_GO { "go", 'g', cmd_go, " time [right=50] [left=right]"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_GO,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 ); // zero add means no real device


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  motor_dir.reset( 0x1F );

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

  return 0;
}

int cmd_go( int argc, const char * const * argv )
{
  int t    = arg2long_d( 1, argc, argv, 1000,  0, 10000 );
  int r_w  = arg2long_d( 2, argc, argv,   50, -100, 100 );
  int l_w  = arg2long_d( 2, argc, argv,  r_w, -100, 100 );
  pr( NL "go: t= " ); pr_d( t ); pr( " r= " ); pr_d( r_w ); pr( " l= " ); pr_d( l_w ); pr ( NL );

  uint8_t bits = r_w > 0 ?    1 : 0;
  bits        |= r_w < 0 ?    2 : 0;
  bits        |= l_w > 0 ?    8 : 0;
  bits        |= l_w < 0 ? 0x10 : 0;

  motor_dir.write( bits );

  for( ; t > 0 && !break_flag; t -= go_tick ) {
    delay_ms( t > go_tick ? go_tick : t );
  }

  motor_dir.reset( motor_bits );
  if( break_flag ) {
    pr( "Break!" NL );
  }

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc


