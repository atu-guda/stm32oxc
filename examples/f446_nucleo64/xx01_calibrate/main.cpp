#define _GNU_SOURCE
#include <cmath>
#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;



BOARD_DEFINE_LEDS_EX;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {

void task_main( void *prm UNUSED_ARG );


}


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
  delay_bad_ms( 1000 );
  leds.write( 1 );
  delay_bad_ms( 1000 );
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

  UVAR('t') = 1000;
  UVAR('n') = 10;
  // UVAR('x') = 10000000; // for calibrate
  uint32_t c_v0 = delay_calibrate_value;
  UVAR('x') = c_v0 * 500; // for calibrate 500 ms

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

  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( NL );

  pr( "Current delay_calibrate_value= " ); pr_d( delay_calibrate_value ); pr( NL );
  do_delay_calibrate();
  pr( "New delay_calibrate_value= " ); pr_d( delay_calibrate_value ); pr( NL );

  UVAR('z') = _write( 1, "ABCDEFGH", 6 );
  putchar( 'X' );
  puts( "012345" );
  _write( 1, "ZZZ\r\n", 5 );
  printf( "M_PI= %.12g" NL, M_PI );


  TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  uint32_t tm0 = HAL_GetTick();
  // delay_bad_n( UVAR('x') );
  // // delay_bad_n( 335730317 ); // for 168 Mhz 10 s
  // // delay_bad_n( 33539491 );     // for 168 Mhz 1 s
  // // delay_bad_n( 33539 );     // for 168 Mhz 1 ms
  // uint32_t tm1 = HAL_GetTick(), dlt = tm1 - tm0;
  // uint32_t c_v = UVAR('x') / dlt;
  // pr( "n= "); pr_d( UVAR('x') ); pr( "  delta= " ); pr_d( dlt ); pr( "  c_v= " ); pr_d( c_v ); pr( NL );

  for( int i=0; i<n && !break_flag; ++i ) {
    TickType_t tcc = xTaskGetTickCount();
    uint32_t tmc = HAL_GetTick();
    pr( " Fake Action i= " ); pr_d( i );
    pr( "  tick: "); pr_d( tcc - tc00 );
    pr( "  ms_tick: "); pr_d( tmc - tm0 );
    pr( NL );
    // vTaskDelayUntil( &tc0, t_step );
    delay_bad_ms( t_step );
    // delay_ms( t_step );
  }

  return 0;
}


//  ----------------------------- configs ----------------



FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

