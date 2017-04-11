#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>
#include <oxc_pcf8591.h>

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
  DEBUG_I2C_CMDS,

  &CMDINFO_TEST0,
  nullptr
};


extern "C" {
void task_main( void *prm UNUSED_ARG );
}

I2C_HandleTypeDef i2ch;
PCF8591 adc( &i2ch );

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
  i2c_dbg = &adc;

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
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( NL );

  int v_end = UVAR('e');

  adc.setMode( PCF8591::autoinc | PCF8591::mode_4in | PCF8591::out_en );

  TickType_t tc0 = xTaskGetTickCount();

  break_flag = 0;
  constexpr const int n_ch = 4;
  uint8_t d_in[n_ch] = { 0, 0, 0, 0 };

  for( int i=0; i<n && !break_flag ; ++i ) {

    adc.getIn( d_in, n_ch );
    pr( "[" ); pr_d( i ); pr( "]  " );
    for( int j=0; j<n_ch; ++j ) {
      pr_d( d_in[j] ); pr( "  " );
    }
    adc.setOut( i & 0xFF );
    pr( NL );
    vTaskDelayUntil( &tc0, t_step );
    // delay_ms( t_step );
  }
  adc.setOut( v_end );

  pr( NL );

  delay_ms( 10 );
  break_flag = 0;  idle_flag = 1;

  pr( NL "test0 end." NL );
  return 0;
}

//  ----------------------------- configs ----------------

FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

