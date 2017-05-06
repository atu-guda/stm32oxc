#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

void MX_GPIO_Init(void);
int MX_USART1_UART_Init(void);

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;


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
UsartIO usartio( &uah, USART1 );
STD_USART1_SEND_TASK( usartio );
// STD_USART1_RECV_TASK( usartio );
STD_USART1_IRQ( usartio );

int main(void)
{
  HAL_Init();

  leds.initHW();
  leds.write( BOARD_LEDS_ALL );

  int rc = SystemClockCfg();
  if( rc ) {
    die4led( BOARD_LEDS_ALL );
    return 0;
  }

  leds.write( 0x0A );  delay_bad_ms( 200 );

  if( ! MX_USART1_UART_Init() ) {
      die4led( 1 );
  }
  HAL_UART_Transmit( &uah, (uint8_t*)"START\r\n", 7, 100 );

  usartio.sendStrSync( "0123456789---main()---ABCDEF" NL );
  leds.write( 0x00 );

  UVAR('t') = 1000;
  UVAR('n') = 10;

  global_smallrl = &srl;

  //           code               name    stack_sz      param  prty TaskHandle_t*
  xTaskCreate( task_leds,        "leds", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_usart1_send, "send", 1*def_stksz, nullptr,   2, nullptr );  // 2
  xTaskCreate( task_main,        "main", 1*def_stksz, nullptr,   1, nullptr );
  xTaskCreate( task_gchar,      "gchar", 2*def_stksz, nullptr,   1, nullptr );

  leds.write( 0x00 );
  ready_to_start_scheduler = 1;
  vTaskStartScheduler();

  die4led( 0xFF );
  return 0;
}

void task_main( void *prm UNUSED_ARG ) // TMAIN
{
  SET_UART_AS_STDIO(usartio);

  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}





// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

