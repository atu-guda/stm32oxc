#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>

#include <oxc_ssd1306.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test ssd1306 based OLED screen" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', " - test basic actions"  );
DCL_CMD_REG( send, 'S', "XX [XX] - send cmd [with arg]"  ); // debug ctrl
DCL_CMD_REG( cls, 'X', " - clear screen"  );
DCL_CMD_REG( vline, 0, " [start [end]] - test vline"  );
DCL_CMD_REG( lines, 'L', " [n [d]] - test lines"  );
DCL_CMD_REG( line, 'l', " [x0 y0 x1 y1] - test line"  );
DCL_CMD_REG( puts, 's', " str [ x y szi] - test string"  );
DCL_CMD_REG( pix, 'x', " x y col - test pixel"  );
DCL_CMD_REG( contr,  0, " [v] - test contrast"  );



I2C_HandleTypeDef i2ch;

const uint16_t xmax = SSD1306::X_SZ, ymax = SSD1306::Y_SZ_MAX;
uint16_t xcen = xmax/2, ycen = ymax/2, ystp = ymax / 10;
PixBuf1V pb0( xmax, ymax );

DevI2C i2cd( &i2ch, 0 );
SSD1306 screen( i2cd );


int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 1000;
  UVAR_n = 20;
  UVAR_y =  0; // 1= y = 32, 0 - 64

  UVAR_e = i2c_default_init( i2ch, 400000 );
  i2c_dbg = &i2cd;
  i2c_client_def = &screen;

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
  int dly = UVAR_t;
  screen.init( UVAR_y );
  ycen = screen.getYsz() / 2; ystp = ycen / 5;

  delay_ms( dly );
  screen.contrast( 0x20 );
  delay_ms( dly );
  screen.contrast( 0xF0 );
  delay_ms( dly );
  // screen.full_on();
  // delay_ms( dly );
  // screen.on_ram();
  //
  // delay_ms( dly );
  screen.inverse();
  delay_ms( dly );
  screen.no_inverse();
  //
  //
  // delay_ms( dly );
  // screen.switch_off();
  // delay_ms( dly );
  screen.switch_on();

  // screen.mode_horisontal();
  // screen.mode_vertical();


  pb0.fillAll( 0 );

  // uint8_t *fb = screen.fb();

  // pb0.fillAll( 1 );
  for( uint16_t co = 0; co < ymax; ++ co ) {
    pb0.pix( co,   co, 1 );
    pb0.pix( 2*co, co, 1 );
  }
  pb0.pix(   ystp,   ystp, 0 );
  pb0.pix( 4*ystp, 4*ystp, 0 );

  pb0.hline(      0,    0,        xmax,   1 );
  pb0.hline( 1*ystp, ystp,   xmax-1*ystp, 1 );
  pb0.hline( 3*ystp, ystp,   xmax-3*ystp, 0 );

  for( uint16_t ro=0; ro<ycen; ++ro ) {
    pb0.vline( ro, 0, ro, (ro&1) );
  }

  pb0.box(   3,   3,  xmax-3,  4*ystp-3, 1 );
  pb0.box(  17,   7,  xmax-7,  4*ystp-7, 0 );
  pb0.rect( 24,   5,      34,  4*ystp-5, 0 );
  pb0.rect( 37,   5,      47,  4*ystp-5, 1 );

  pb0.fillCircle( xcen, ycen, 5*ystp, 1 );
  pb0.fillCircle( xcen, ycen, 2*ystp, 0 );
  pb0.circle( xcen, ycen, 3*ystp, 0 );
  pb0.circle( xcen, ycen, 1*ystp, 1 );

  pb0.outChar( 2, 2, 'A', &Font12,  0 );
  pb0.outStrBox(      2*ystp,  7*ystp, "String8", &Font8,  1, 0, 1, PixBuf::STRBOX_ALL );
  pb0.outStrBox( xmax-10*ystp, 7*ystp, "Str16",   &Font16, 0, 1, 0, PixBuf::STRBOX_BG );

  screen.out( pb0 );

  return 0;
}

int cmd_send( int argc, const char * const * argv )
{
  const unsigned max_cmd_bytes { 8 };
  uint8_t ca[max_cmd_bytes];
  int n_cmd = argc-1;
  for( int i=0; i<n_cmd; ++i ) {
    ca[i] = strtol( argv[i+1], 0, 0 );
  }
  // dump8( ca, n_cmd );
  screen.cmdN( ca, n_cmd );

  screen.out( pb0 );

  return 0;
}


int cmd_cls( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  pb0.fillAll( 0 );
  screen.out( pb0 );

  return 0;
}


int cmd_vline( int argc, const char * const * argv )
{
  uint16_t y0 = arg2long_d( 1, argc, argv,    0,  0, ymax );
  uint16_t y1 = arg2long_d( 2, argc, argv, ymax, y0, ymax );
  std_out <<  NL "vline: y0= "  <<  y0  <<  " y1= " << y1;

  for( uint16_t y = y0; y<= y1; ++y ) {
    pb0.vline( y, y/2, y, (y&1) );
  }

  screen.out( pb0 );

  return 0;
}

int cmd_lines( int argc, const char * const * argv )
{
  uint16_t nl = arg2long_d( 1, argc, argv,  36, 0, 1024 );
  uint16_t dn = arg2long_d( 2, argc, argv,  360/nl, 1, 512 );

  std_out <<  NL "lines: nl = "  <<  nl  <<  " dn= "  <<  dn;

  pb0.box( xcen, ystp, xmax-ystp, ymax-ystp, 1 );

  for( uint16_t an = 0;  an <= nl; ++an ) {
    float alp = an * dn * 3.1415926f / 180;
    uint16_t x1 = xcen + 8 * ystp * cosf( alp );
    uint16_t y1 = ycen + 8 * ystp * sinf( alp );
    // std_out <<  "x1= "  <<  x1 <<  " y1= "  <<  y1;
    pb0.line( xcen, ycen, x1, y1, (an&1) );
  }
  std_out << NL;

  screen.out( pb0 );

  return 0;
}

int cmd_line( int argc, const char * const * argv )
{
  uint16_t x0 = arg2long_d( 1, argc, argv,  0,  0, 512 );
  uint16_t y0 = arg2long_d( 2, argc, argv,  0,  0, 512 );
  uint16_t x1 = arg2long_d( 3, argc, argv, 64,  0, 512 );
  uint16_t y1 = arg2long_d( 4, argc, argv, 64,  0, 512 );

  std_out <<  NL "line: "  << x0 << ' ' << y0 << ' ' << x1 << ' ' << y1  <<  NL;


  pb0.line( x0, y0, x1, y1, 1 );

  screen.out( pb0 );

  return 0;
}

int cmd_puts( int argc, const char * const * argv )
{
  static const sFONT* fonts[] = { &Font8, &Font12, &Font16, &Font20, &Font24 };
  const char *s = argv[1] ? argv[1] : "W";
  uint16_t x0  = arg2long_d( 2, argc, argv,  0,  0, 512 );
  uint16_t y0  = arg2long_d( 3, argc, argv,  0,  0, 512 );
  uint16_t sid = arg2long_d( 4, argc, argv,  0,  0, size(fonts)-1 );

  pb0.outStr( x0, y0, s, fonts[sid], 1 );

  screen.out( pb0 );

  return 0;
}

int cmd_pix( int argc, const char * const * argv )
{
  uint16_t x0  = arg2long_d( 1, argc, argv,  0,  0, 512 );
  uint16_t y0  = arg2long_d( 2, argc, argv,  0,  0, 512 );
  uint16_t c   = arg2long_d( 3, argc, argv,  1,  0, 255 );

  pb0.pix( x0, y0, c );

  screen.out( pb0 );

  return 0;
}



int cmd_contr( int argc, const char * const * argv )
{
  uint8_t co = arg2long_d( 1, argc, argv,  0x48, 0, 0x7F );
  std_out <<  NL "contrast: co="  <<  co << NL;

  screen.contrast( co );

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

