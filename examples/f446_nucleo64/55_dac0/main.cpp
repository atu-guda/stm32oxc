#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;



BOARD_DEFINE_LEDS_EX;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

SmallRL srl( smallrl_exec );

extern DAC_HandleTypeDef hdac;
int MX_DAC_Init();

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

  UVAR('e') = MX_DAC_Init();

  leds.write( 0x05 );  delay_bad_ms( 200 );

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

  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t v1 = arg2long_d( 1, argc, argv, UVAR('v'), 0 );
  uint32_t v2 = v1;
  int n = arg2long_d( 2, argc, argv, UVAR('n'), 0 );
  pr( NL "Test0: n= " ); pr_d( n ); pr( " v1= " ); pr_d( v1 ); pr( " v2= " ); pr_d( v2 );
  pr( NL );

  auto r1 = HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, v1 );
  auto r2 = HAL_DAC_SetValue( &hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, v2 );

  pr( "r1= " ); pr_d( r1 ); pr( "  r2= " ); pr_d( r2 ); pr( NL );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_2 );
  uint32_t vv = 3250 * v1 / 4096;
  pr_sdx( vv );

  // int prty = uxTaskPriorityGet( 0 );
  // pr_sdx( prty );
  // const char *nm = pcTaskGetName( 0 );
  // pr( "name: \"" ); pr( nm ); pr( "\"" NL );
  //
  // // log_add( "Test0 " );
  // TickType_t tc0 = xTaskGetTickCount(), tc00 = tc0;
  // uint32_t tm0 = HAL_GetTick();
  //
  // break_flag = 0;
  // for( int i=0; i<n && !break_flag; ++i ) {
  //   TickType_t tcc = xTaskGetTickCount();
  //   uint32_t tmc = HAL_GetTick();
  //   pr( " Fake Action i= " ); pr_d( i );
  //   pr( "  tick: "); pr_d( tcc - tc00 );
  //   pr( "  ms_tick: "); pr_d( tmc - tm0 );
  //   pr( NL );
  //   vTaskDelayUntil( &tc0, t_step );
  //   // delay_ms( t_step );
  // }

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}



//  ----------------------------- configs ----------------



FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

