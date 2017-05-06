#include <cstring>
#include <cstdlib>
#include <cmath>

#include <algorithm>

#include <oxc_auto.h>

#include <oxc_pcd8544.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;


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


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

int MX_SPI2_Init( uint32_t prescal = SPI_BAUDRATEPRESCALER_64 );
PinsOut nss_pin( GPIOB, 12, 1 );
PinsOut rst_dc_pins( GPIOB, 10, 2 );
SPI_HandleTypeDef spi2_h;
DevSPI spi_d( &spi2_h, &nss_pin );

const uint16_t xmax = PCD8544::X_SZ, ymax = PCD8544::Y_SZ;
const uint16_t xcen = xmax/2, ycen = ymax/2, ystp = ymax / 10;
PixBuf1V pb0( xmax, ymax );
PCD8544 screen( spi_d, rst_dc_pins );


int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 1000;
  UVAR('n') = 10;

  MX_SPI2_Init();
  screen.init();

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
  pr( NL "vline: y0= " ); pr_d( y0 ); pr( " y1= " ); pr_h( y1 );

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
  pr( NL "lines: nl = " ); pr_d( nl ); pr( " dn= " ); pr_d( dn );

  pb0.box( xcen, ystp, xmax-ystp, ymax-ystp, 1 );

  for( uint16_t an = 0;  an <= nl; ++an ) {
    float alp = an * dn * 3.1415926f / 180;
    uint16_t x1 = xcen + 8 * ystp * cosf( alp );
    uint16_t y1 = ycen + 8 * ystp * sinf( alp );
    // pr( "x1= " ); pr_d( x1 );
    // pr( " y1= " ); pr_d( y1 ); pr( NL );
    pb0.line( xcen, ycen, x1, y1, (an&1) );
  }

  screen.out( pb0 );

  return 0;
}

int cmd_contr( int argc, const char * const * argv )
{
  uint8_t co = arg2long_d( 1, argc, argv,  0x48, 0, 0x7F );
  pr( NL "contrast: co=" ); pr_d( co );

  screen.contrast( co );

  return 0;
}


//  ----------------------------- configs ----------------


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

