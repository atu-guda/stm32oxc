#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <oxc_auto.h>
#include <oxc_hd44780_i2c.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;
// PinsOut leds( BOARD_LEDS_GPIO, BOARD_LEDS_OFS, BOARD_N_LEDS );

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App measure and control analog and digital signals" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_setaddr( int argc, const char * const * argv );

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
HD44780_i2c lcdt( i2cd, 0x27 );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100;
  UVAR('n') = 10;

  UVAR('e') = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;

  lcdt.init_4b();
  lcdt.cls();
  lcdt.putch( 'X' );
  lcdt.puts( " ptn-hlo!" );

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
  int n  = arg2long_d( 1, argc, argv, UVAR('n'), 1,   100000000 );
  uint32_t t_step = UVAR('t');


  STDOUT_os;
  os << "# n= " << n << " t= " << t_step << NL; os.flush();

  char buf0[32], buf1[32];

  const int n_adc_ch = 4;
  float vf[n_adc_ch];
  char adc_txt_bufs[n_adc_ch][16];
  const int n_din_ch = 4;
  int  d_in[n_din_ch];

  bool show_lcd = true;
  if( t_step < 50 ) {
    show_lcd = false;
    lcdt.cls();
    lcdt.puts( "t < 50 ms!  " );
  }

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    // fake data
    for( int j=0; j<n_adc_ch; ++j ) {
      vf[j] = 0.001f * ( i % 1000 ) + j;
    }
    for( int j=0; j<n_din_ch; ++j ) {
      d_in[j] = ( i & (1<<j) ) ? 1 : 0;
    }

    for( int j=0; j<n_adc_ch; ++j ) {
      snprintf( adc_txt_bufs[j],   sizeof(adc_txt_bufs[j]),  "%6.4f ", vf[j] );
    }


    uint32_t ct = HAL_GetTick();
    os << i << ' ' << ( ct - tm00 ) << ' ';
    for( int j=0; j<n_adc_ch; ++j ) {
      os << adc_txt_bufs[j] << ' ';
    }
    for( int j=0; j<n_din_ch; ++j ) {
      os << d_in[j] << ' ';
    }
    os << NL;

    if( show_lcd ) {
      strcpy( buf0, adc_txt_bufs[0] );
      strcat( buf0, adc_txt_bufs[1] );
      strcpy( buf1, adc_txt_bufs[2] );
      strcat( buf1, adc_txt_bufs[3] );
      buf0[14] = d_in[0] ? '$' : '.';
      buf0[15] = d_in[1] ? '$' : '.';
      buf1[14] = d_in[2] ? '$' : '.';
      buf1[15] = d_in[3] ? '$' : '.';
      buf0[16] = '\0';  buf1[16] = '\0';
      lcdt.gotoxy( 0, 0 );
      lcdt.puts( buf0 );
      lcdt.gotoxy( 0, 1 );
      lcdt.puts( buf1 );
    }

    leds.toggle( BIT1 );



    os.flush();
    delay_ms_until_brk( &tm0, t_step );
  }

  pr( NL );

  return 0;
}

// int cmd_xychar( int argc, const char * const * argv )
// {
//   uint8_t x  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   64 );
//   uint8_t y  = (uint8_t)arg2long_d( 2, argc, argv, 0x0, 0,    3 );
//   uint8_t ch = (uint8_t)arg2long_d( 3, argc, argv, 'Z', 0, 0xFF );
//
//   // lcdt.gotoxy( x, y );
//   lcdt.putxych( x, y, (uint8_t)ch );
//
//   return 0;
// }
//
// int cmd_gotoxy( int argc, const char * const * argv )
// {
//   uint8_t x  = (uint8_t)arg2long_d( 1, argc, argv, 0x0, 0,   64 );
//   uint8_t y  = (uint8_t)arg2long_d( 2, argc, argv, 0x0, 0,    3 );
//
//   lcdt.gotoxy( x, y );
//   return 0;
// }
//
// int cmd_puts( int argc, const char * const * argv )
// {
//   const char *s = "Z";
//   if( argc > 1 ) {
//     s = argv[1];
//   }
//   lcdt.puts( s );
//
//   return 0;
// }


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

