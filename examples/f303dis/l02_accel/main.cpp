#include <cstring>
#include <cstdlib>

#include <oxc_gpio.h>
#include <oxc_usartio.h>
#include <oxc_console.h>
#include <oxc_debug1.h>
#include <oxc_lsm303dlhc_accel.h>
#include <oxc_debug_i2c.h>
#include <oxc_common1.h>
#include <oxc_smallrl.h>

#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>

using namespace std;
using namespace SMLRL;

// PinsOut p1 { GPIOE, 8, 8 };
BOARD_DEFINE_LEDS;



const int def_stksz = 1 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
LSM303DHLC_Accel accel( &i2ch );
void MX_I2C1_Init( I2C_HandleTypeDef &i2c );

void draw_hbar_gauge( int x, int y, int w, int v, int vmin, int vmax, int fd = 1 );

UART_HandleTypeDef uah;
UsartIO usartio( &uah, USART2 );
int init_uart( UART_HandleTypeDef *uahp, int baud = 115200 );

STD_USART2_SEND_TASK( usartio );
// STD_USART2_RECV_TASK( usartio );
STD_USART2_IRQ( usartio );

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

  MX_I2C1_Init( i2ch );
  i2c_dbg = &accel;

  leds.write( 0x00 );

  UVAR('t') = 100;
  UVAR('n') = 50;

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
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  int scale = arg2long_d( 2, argc, argv, 0, 0, 3 );
  uint32_t t_step = UVAR('t');
  const uint8_t c4_scales[] = {
    LSM303DHLC_Accel::Ctl4_val::scale_2g,
    LSM303DHLC_Accel::Ctl4_val::scale_4g,
    LSM303DHLC_Accel::Ctl4_val::scale_8g,
    LSM303DHLC_Accel::Ctl4_val::scale_16g,
    0
  };

  uint8_t c4v = c4_scales[scale];
  if( argc > 3 ) {
    c4v |= (uint8_t)LSM303DHLC_Accel::Ctl4_val::hr_enable;
  }

  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( " scale= " ); pr_d( scale ); pr( " c4v= " ); pr_h( c4v );
  pr( NL );

  accel.resetDev();

  // if( ! accel.check_id() ) {
  //   pr( "LSM303DHLC_Accel no found" NL );
  //   return 1;
  // }
  if( ! accel.init( (LSM303DHLC_Accel::Ctl4_val)c4v ) ) {
    pr( "Fail to init LSM303DHLC_Accel" NL );
    return 1;
  }
  accel.rebootMem(); // bad try to drop previous measurement

  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  int16_t xyz[3];

  // term_set_scroll_area( 2, 16 );
  // pr( NL );
  term_clear();
  // delay_ms( 50 );

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    accel.getAccAll( xyz );
    TickType_t tcc = xTaskGetTickCount();
    draw_hbar_gauge( 1, 1, 100, xyz[0], INT16_MIN, INT16_MAX );
    draw_hbar_gauge( 1, 2, 100, xyz[1], INT16_MIN, INT16_MAX );
    draw_hbar_gauge( 1, 3, 100, xyz[2], INT16_MIN, INT16_MAX );
    term_set_xy( 10, 5 );
    pr( "i= " ); pr_d( i );
    pr( "  tick: "); pr_d( tcc - tc00 );
    pr( " [ " ); pr_d( xyz[0] );
    pr( " ; " ); pr_d( xyz[1] );
    pr( " ; " ); pr_d( xyz[2] );
    pr( " ] " /* NL  */ );
    vTaskDelayUntil( &tc0, t_step );
  }

  // term_set_scroll_area( -1, -1 );

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

void draw_hbar_gauge( int x, int y, int w, int v, int vmin, int vmax, int fd )
{
  int w0 = w - 2;
  if( w0 < 3 ) { return; };
  char lbra = '[', rbra = ']';
  if( v < vmin ) { v = vmin; lbra = '$'; };
  if( v > vmax ) { v = vmax; rbra = '$'; };

  int iv = ( v - vmin ) * w0 / ( vmax - vmin );
  int i0 = ( 0 - vmin ) * w0 / ( vmax - vmin );
  char zero_char = '*';
  if( i0 < 0 )   { i0 = 0;    zero_char = '<'; };
  if( i0 >= w0 ) { i0 = w0-1; zero_char = '>'; };

  int lb = i0, rb = iv;
  bool left_zero = true;
  if( lb > rb ) { int t = lb; lb = rb; rb = t; left_zero = false; }
  term_save_cpos( fd );
  term_set_xy( x, y, fd );


  pr_c( lbra, fd );
  int i; // not in loop
  for( i=0; i<lb-1; ++i ) {
    pr_c( '.', fd );
  }
  pr_c( left_zero ? zero_char : '.' ); ++i;
  for( /* */; i<rb-2; ++i ) {
    pr_c( 'W', fd );
  }
  pr_c( left_zero ? 'W' : zero_char ); ++i;
  for( /* */; i<w; ++i ) {
    pr_c( '.', fd );
  }
  pr_c( rbra, fd );

  term_rest_cpos( fd );
}
//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

