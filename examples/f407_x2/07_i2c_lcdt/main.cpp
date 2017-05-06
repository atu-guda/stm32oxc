#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_hd44780_i2c.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_setaddr( int argc, const char * const * argv );
CmdInfo CMDINFO_SETADDR { "setaddr", 0, cmd_setaddr, " addr - set device addr (see 'C')"  };
int cmd_gotoxy( int argc, const char * const * argv );
CmdInfo CMDINFO_GOTOXY{ "gotoxy", 'G', cmd_gotoxy, " x y - move pos to (x, y)"  };
int cmd_xychar( int argc, const char * const * argv );
CmdInfo CMDINFO_XYCHAR{ "xychar", 'X', cmd_xychar, " x y code - put char at x y"  };
int cmd_puts( int argc, const char * const * argv );
CmdInfo CMDINFO_PUTS{ "puts", 'P', cmd_puts, "string - put string at cur pos"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_SETADDR,
  &CMDINFO_XYCHAR,
  &CMDINFO_GOTOXY,
  &CMDINFO_PUTS,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x27 );

void MX_I2C1_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  MX_I2C1_Init( i2ch );
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
  // int n = UVAR('n');
  uint32_t t_step = UVAR('t');
  uint16_t ch_st = (uint8_t)arg2long_d( 1, argc, argv, 0x30, 0, 256-16 );
  uint16_t ch_en = ch_st + 0x10;

  lcdt.init_4b();
  int state = lcdt.getState();
  pr_sdx( state );

  lcdt.cls();
  lcdt.putch( 'X' );
  lcdt.puts( " ptn-hlo!\n\t" );
  lcdt.curs_on();
  delay_ms( t_step );
  lcdt.off();
  delay_ms( t_step );
  lcdt.led_off();
  delay_ms( t_step );
  lcdt.led_on();
  delay_ms( t_step );
  lcdt.on();
  lcdt.gotoxy( 2, 1 );
  for( uint16_t ch = ch_st; ch < ch_en; ++ch ) {
    lcdt.putch( (uint8_t)ch );
  }

  pr( NL );

  return 0;
}

int cmd_setaddr( int argc, const char * const * argv )
{
  if( argc < 2 ) {
    pr( "Need addr [1-127]" NL );
    return 1;
  }
  uint8_t addr  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   127 );
  lcdt.setAddr( addr );
  return 0;
}

int cmd_xychar( int argc, const char * const * argv )
{
  uint8_t x  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   64 );
  uint8_t y  = (uint8_t)arg2long_d( 2, argc, argv, 0x0, 0,    3 );
  uint8_t ch = (uint8_t)arg2long_d( 3, argc, argv, 'Z', 0, 0xFF );

  // lcdt.gotoxy( x, y );
  lcdt.putxych( x, y, (uint8_t)ch );

  return 0;
}

int cmd_gotoxy( int argc, const char * const * argv )
{
  uint8_t x  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   64 );
  uint8_t y  = (uint8_t)arg2long_d( 2, argc, argv, 0x0, 0,    3 );

  lcdt.gotoxy( x, y );
  return 0;
}

int cmd_puts( int argc, const char * const * argv )
{
  const char *s = "Z";
  if( argc > 1 ) {
    s = argv[1];
  }
  lcdt.puts( s );

  return 0;
}

//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

