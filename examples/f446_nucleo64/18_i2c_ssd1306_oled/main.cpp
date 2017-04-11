#include <cstring>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>

#include <oxc_ssd1306.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

BOARD_DEFINE_LEDS_EX;



const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

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
  DEBUG_I2C_CMDS,

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

I2C_HandleTypeDef i2ch;

const uint16_t xmax = SSD1306::X_SZ, ymax = SSD1306::Y_SZ;
const uint16_t xcen = xmax/2, ycen = ymax/2, ystp = ymax / 10;
PixBuf1V pb0( xmax, ymax );
SSD1306 screen( &i2ch );

void MX_I2C1_Init( I2C_HandleTypeDef &i2c, uint32_t speed = 100000 );


UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART2 );
void init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART2_SEND_TASK( usartio );
// STD_USART2_RECV_TASK( usartio );
STD_USART2_IRQ( usartio );

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL_EX );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL_EX );
    return 0;
  }

  HAL_Delay( 200 ); // delay_bad_ms( 200 );
  leds.write( 0x00 ); delay_ms( 200 );
  leds.write( BOARD_LEDS_ALL_EX );  HAL_Delay( 200 );

  init_uart( &uah );
  leds.write( 0x0A );  delay_bad_ms( 200 );

  MX_I2C1_Init( i2ch );
  i2c_dbg = &screen;

  delay_bad_ms( 200 );  leds.write( 1 );


  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 2*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 2*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_UART_AS_STDIO( usartio );

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  pr( NL "Test0: " NL );

  screen.init();

  delay_ms( 500 );
  screen.contrast( 0x20 );
  delay_ms( 500 );
  screen.contrast( 0xF0 );
  delay_ms( 500 );
  // screen.full_on();
  // delay_ms( 500 );
  // screen.on_ram();
  //
  // delay_ms( 500 );
  screen.inverse();
  delay_ms( 500 );
  screen.no_inverse();
  //
  //
  // delay_ms( 500 );
  // screen.switch_off();
  // delay_ms( 1000 );
  screen.switch_on();

  screen.mode_horisontal();


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

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;
  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

int cmd_cls( int argc UNUSED_ARG, const char * const * argv UNUSED_ARG )
{
  pb0.fillAll( 0 );
  screen.out( pb0 );
  pr( NL "CLS end." NL );
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

  pr( NL "vline end." NL );
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

  pr( NL "lines end." NL );
  return 0;
}

int cmd_contr( int argc, const char * const * argv )
{
  uint8_t co = arg2long_d( 1, argc, argv,  0x48, 0, 0x7F );
  pr( NL "contrast: co=" ); pr_d( co );

  screen.contrast( co );

  pr( NL "contrast end." NL );
  return 0;
}


//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

