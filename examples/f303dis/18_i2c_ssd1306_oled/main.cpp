#include <cstring>
#include <cstdlib>
#include <algorithm>

#include <oxc_gpio.h>
#include <oxc_usartio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_debug_i2c.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

#include <oxc_ssd1306.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOE, 8, 8 };
BOARD_DEFINE_LEDS;



const int def_stksz = 1 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_print, smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

int cmd_cls( int argc, const char * const * argv );
CmdInfo CMDINFO_CLS { "cls", 'X', cmd_cls, " - clear screen"  };

int cmd_vline( int argc, const char * const * argv );
CmdInfo CMDINFO_VLINE { "vline", 0, cmd_vline, " [start [end]] - test vline"  };

int cmd_line( int argc, const char * const * argv );
CmdInfo CMDINFO_LINE { "line", 'L', cmd_line, " - test line"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_CLS,
  &CMDINFO_VLINE,
  &CMDINFO_LINE,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
void MX_I2C1_Init( I2C_HandleTypeDef &i2c );

UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART2 );
int init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART2_SEND_TASK( usartio );
// STD_USART2_RECV_TASK( usartio );
STD_USART2_IRQ( usartio );
// ----------------------------------------------------------------

PixBuf1V pb0( 128, 64 );
SSD1306 screen( i2ch );


int main(void)
{
  HAL_Init();

  SystemClock_Config();
  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  if( !init_uart( &uah ) ) {
    die4led( 0x08 );
  }
  leds.write( 0x0A );  delay_bad_ms( 200 );

  // HAL_UART_Transmit( &uah, (uint8_t*)"START\r\n", 7, 100 );

  usartio.sendStrSync( "0123456789---main()---ABCDEF" NL );

  MX_I2C1_Init( i2ch );
  i2ch_dbg = &i2ch;


  leds.write( 0x00 );

  user_vars['t'-'a'] = 1000;
  user_vars['n'-'a'] = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart2_send, "send", 1*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  vTaskStartScheduler();
  die4led( 0xFF );



  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  uint32_t nl = 0;

  delay_ms( 50 );
  user_vars['t'-'a'] = 1000;

  usartio.itEnable( UART_IT_RXNE );
  usartio.setOnSigInt( sigint );
  devio_fds[0] = &usartio; // stdin
  devio_fds[1] = &usartio; // stdout
  devio_fds[2] = &usartio; // stderr

  delay_ms( 50 );
  pr( "*=*** Main loop: ****** " NL );
  delay_ms( 20 );

  srl.setSigFun( smallrl_sigint );
  srl.set_ps1( "\033[32m#\033[0m ", 2 );
  srl.re_ps();
  srl.set_print_cmd( true );


  idle_flag = 1;
  while(1) {
    ++nl;
    if( idle_flag == 0 ) {
      pr_sd( ".. main idle  ", nl );
      srl.redraw();
    }
    idle_flag = 0;
    delay_ms( 60000 );
    // delay_ms( 1 );

  }
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  pr( NL "Test0:  " NL );

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
  delay_ms( 500 );
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
  for( uint16_t co = 0; co < 64; ++ co ) {
    pb0.pix( co,   co, 1 );
    pb0.pix( 2*co, co, 1 );
  }
  pb0.pix( 10,  10, 0 );
  pb0.pix( 40,  20, 0 );

  pb0.hline(  0,  0, 128, 1 );
  pb0.hline( 11, 10, 100, 1 );
  pb0.hline( 80, 10,  90, 0 );
  pb0.hline(  0, 60, 200, 1 );
  pb0.hline(  0, 30, 200, 0 );

  for( uint16_t ro=0; ro<64; ++ro ) {
    pb0.vline( ro, 0, ro, (ro&1) );
  }

  pb0.rect( 10, 10, 117, 53, 1 );
  pb0.rect( 11, 11, 116, 52, 0 );
  pb0.box(  12, 12, 64,  51, 1 );
  pb0.box(  65, 11, 115, 51, 0 );

  pb0.fillCircle( 64, 32, 10, 1 );
  pb0.fillCircle( 64, 32, 6,  0 );
  pb0.circle( 64, 32, 15, 0 );
  pb0.circle( 64, 32, 20, 1 );

  pb0.outChar( 20, 20, 'A', &Font12,  0 );
  pb0.outStrBox( 90, 20, "String", &Font8, 1, 0, 1, PixBuf::STRBOX_ALL );
  pb0.outStrBox( 90, 40, "Str2", &Font8, 0, 1, 0, PixBuf::STRBOX_BG );

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
  uint16_t y0 = arg2long_d( 1, argc, argv,  0, 0, 256 );
  uint16_t y1 = arg2long_d( 2, argc, argv, 64, 0, 256 );
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
  pr( NL "lines: = nl" ); pr_d( nl ); pr( " dn= " ); pr_d( dn );

  uint16_t x0 = 64, y0 = 32;

  for( uint16_t an = 0;  an <= nl; ++an ) {
    float alp = an * dn * 3.1415926 / 180;
    uint16_t x1 = x0 + 30 * cosf(alp);
    uint16_t y1 = y0 + 30 * sinf(alp);
    // pr( "x1= " ); pr_d( x1 );
    // pr( " y1= " ); pr_d( y1 ); pr( NL );
    pb0.line( x0, y0, x1, y1, (an&1) );
  }

  screen.out( pb0 );

  pr( NL "lines end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

