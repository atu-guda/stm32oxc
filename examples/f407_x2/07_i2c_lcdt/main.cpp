#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_hd44780_i2c.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;


const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_xychar( int argc, const char * const * argv );
CmdInfo CMDINFO_XYCHAR{ "xychar", 'X', cmd_xychar, " x y code - put char at x y"  };
int cmd_puts( int argc, const char * const * argv );
CmdInfo CMDINFO_PUTS{ "puts", 'P', cmd_puts, "string - put string at cur pos"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_XYCHAR,
  &CMDINFO_PUTS,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 ); // zero add means no real device
HD44780_i2c lcdt( &i2ch, 0x3F );

void MX_I2C1_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );


int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  MX_I2C1_Init( i2ch );
  i2c_dbg = &i2cd;

  delay_ms( PROLOG_LED_TIME ); leds.write( 0x01 ); delay_ms( PROLOG_LED_TIME );

  CREATE_STD_TASKS( task_usbcdc_send );

  SCHEDULER_START;
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_USBCDC_AS_STDIO(usbcdc);

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

int cmd_xychar( int argc, const char * const * argv )
{
  uint8_t x  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   64 );
  uint8_t y  = (uint8_t)arg2long_d( 2, argc, argv, 0x0, 0,    3 );
  uint8_t ch = (uint8_t)arg2long_d( 3, argc, argv, 'Z', 0, 0xFF );

  lcdt.gotoxy( x, y );
  lcdt.putch( (uint8_t)ch );

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


// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

