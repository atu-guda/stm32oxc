#include <cmath>

#include <algorithm>

#include <oxc_auto.h>

#include <oxc_pcd8544.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test PCD8544 (nokia 3311) LCD screen" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_cls( int argc, const char * const * argv );
CmdInfo CMDINFO_CLS { "cls", 'X', cmd_cls, " - clear screen"  };

int cmd_vline( int argc, const char * const * argv );
CmdInfo CMDINFO_VLINE { "vline", 0, cmd_vline, " [start [end]] - test vline"  };

int cmd_line( int argc, const char * const * argv );
CmdInfo CMDINFO_LINE { "line", 'L', cmd_line, " - test line"  };

int cmd_contr( int argc, const char * const * argv );
CmdInfo CMDINFO_CONTR { "contr",  0, cmd_contr, " [v] - test contrast"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_CLS,
  &CMDINFO_VLINE,
  &CMDINFO_LINE,
  &CMDINFO_CONTR,
  nullptr
};



PinOut nss_pin( BOARD_SPI_DEFAULT_GPIO_SNSS, BOARD_SPI_DEFAULT_GPIO_PIN_SNSS );
PinOut rst_pin( BOARD_SPI_DEFAULT_GPIO_EXT1, BOARD_SPI_DEFAULT_GPIO_PIN_EXT1 );
PinOut dc_pin(  BOARD_SPI_DEFAULT_GPIO_EXT2, BOARD_SPI_DEFAULT_GPIO_PIN_EXT2 );
SPI_HandleTypeDef spi_h;
DevSPI spi_d( &spi_h, &nss_pin );

const uint16_t xmax = PCD8544::X_SZ, ymax = PCD8544::Y_SZ;
const uint16_t xcen = xmax/2, ycen = ymax/2, ystp = ymax / 10;
PixBuf1V pb0( xmax, ymax );
PCD8544 screen( spi_d, rst_pin, dc_pin );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  if( SPI_init_default( SPI_BAUDRATEPRESCALER_16 ) != HAL_OK ) {
    die4led( 0x04 );
  }
  screen.init();

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
  screen.init();

  screen.full_on();
  delay_ms( 500 );
  // screen.on_ram();
  //
  // delay_ms( 500 );
  screen.inverse();
  delay_ms( 500 );
  screen.no_inverse();
  //
  //
  delay_ms( 500 );
  screen.switch_off();
  delay_ms( 500 );
  screen.switch_on();


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
  pb0.outStrBox(      2*ystp,  7*ystp, "String", &Font8, 1, 0, 1, PixBuf::STRBOX_ALL );
  pb0.outStrBox( xmax-7*ystp,  7*ystp, "Str2",   &Font8, 0, 1, 0, PixBuf::STRBOX_BG );

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
  std_out <<  NL "vline: y0= "  <<  y0  <<  " y1= "  << y1;

  for( uint16_t y = y0; y<= y1; ++y ) {
    pb0.vline( y, y/2, y, (y&1) );
  }

  screen.out( pb0 );

  return 0;
}

int cmd_line( int argc, const char * const * argv )
{
  uint16_t nl = arg2long_d( 1, argc, argv,  36, 0, 1024 );
  uint16_t dn = arg2long_d( 2, argc, argv,  360/nl, 1, 512 );
  std_out <<  NL "lines: nl = "  <<  nl  <<  " dn= "  <<  dn;

  pb0.box( xcen, ystp, xmax-ystp, ymax-ystp, 1 );

  for( uint16_t an = 0;  an <= nl; ++an ) {
    float alp = an * dn * 3.1415926f / 180;
    uint16_t x1 = xcen + 8 * ystp * cosf( alp );
    uint16_t y1 = ycen + 8 * ystp * sinf( alp );
    // std_out <<  "x1= "  <<  x1 <<  " y1= "  <<  y1  <<  NL;
    pb0.line( xcen, ycen, x1, y1, (an&1) );
  }

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

