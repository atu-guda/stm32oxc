#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;

// PinsOut p1 { GPIOC, 0, 4 };
BOARD_DEFINE_LEDS;



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

  SystemClock_Config();

  leds.initHW();

  leds.write( 0x0F );  delay_bad_ms( 200 );
  init_uart( &uah );
  leds.write( 0x0A );  delay_bad_ms( 200 );

  // HAL_UART_Transmit( &uah, (uint8_t*)"START\r\n", 7, 100 );

  // usartio.sendStrSync( "0123456789---main()---ABCDEF" NL );

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
  SET_UART_AS_STDIO(usartio);

  usartio.sendStrSync( "0123456789ABCDEF" NL );
  delay_ms( 10 );

  default_main_loop();
  vTaskDelete(NULL);
}



//  ----------------------------- configs ----------------

void init_uart( UART_HandleTypeDef *uahp, int baud )
{
  uahp->Instance = USART2;
  uahp->Init.BaudRate     = baud;
  uahp->Init.WordLength   = UART_WORDLENGTH_8B;
  uahp->Init.StopBits     = UART_STOPBITS_1;
  uahp->Init.Parity       = UART_PARITY_NONE;
  uahp->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uahp->Init.Mode         = UART_MODE_TX_RX;
  uahp->Init.OverSampling = UART_OVERSAMPLING_16;
  if( HAL_UART_Init( uahp ) != HAL_OK )  {
    die4led( 0x08 );
  }
}


FreeRTOS_to_stm32cube_tick_hook;

// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

