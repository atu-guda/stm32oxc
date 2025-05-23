#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_hd44780_i2c.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test hd4480 LCD screen" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_gotoxy( int argc, const char * const * argv );
CmdInfo CMDINFO_GOTOXY{ "gotoxy", 'G', cmd_gotoxy, " x y - move pos to (x, y)"  };
int cmd_xychar( int argc, const char * const * argv );
CmdInfo CMDINFO_XYCHAR{ "xychar", 'X', cmd_xychar, " x y code - put char at x y"  };
int cmd_puts( int argc, const char * const * argv );
CmdInfo CMDINFO_PUTS{ "puts", 'P', cmd_puts, "string - put string at cur pos"  };
int cmd_lcd_rate( int argc, const char * const * argv );
CmdInfo CMDINFO_LCD_RATE{ "lcd_rate", 0, cmd_lcd_rate, "[N] - measure output rate"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_XYCHAR,
  &CMDINFO_GOTOXY,
  &CMDINFO_PUTS,
  &CMDINFO_LCD_RATE,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x27 );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &lcdt;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
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

// results:
// gotoxy + puts (16) = 13.722 ms/line
// puts (16) = 12.915 ms/line
// gotoxy costs 0.81 ms

int cmd_lcd_rate( int argc, const char * const * argv )
{
  int n  = arg2long_d( 1, argc, argv, 100, 1,  10000 );
  int nl = arg2long_d( 2, argc, argv,   2, 1,      4 );
  std_out << "lcd_rate: n= " << n << " nl= " << nl << NL;
  char buf[32];

  buf[16] = '\0';

  uint32_t tm0 = HAL_GetTick();

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    for( int j=0; j<16; ++j ) {
      buf[j] = (char)( '@' + ( ( j + i ) & 0x3F ) );
    }
    for( int l=0; l<nl; ++l ) {
      if( UVAR('a') < 1 ) {
        lcdt.gotoxy( 0, l );
      }
      lcdt.puts( buf );
      ++buf[0];
      buf[1]  +=  2;
      buf[15] ^=  0x0F;
    }
  }

  uint32_t tm1 = HAL_GetTick();
  delay_ms( 500 );
  int dlt = tm1 - tm0;
  int mcs_l = 1000 * dlt / ( n * nl );
  std_out << NL "dt= " << dlt << " ms, so " << mcs_l << " us/line" NL;
  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

