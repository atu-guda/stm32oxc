#include <cstring>

#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

const int def_stksz = 2 * configMINIMAL_STACK_SIZE;

extern "C" {
void task_leds( void *prm UNUSED_ARG );
void task_send( void *prm UNUSED_ARG );
} // extern "C"

UART_HandleTypeDef uah;

const int TX_BUF_SZ = 128;
char tx_buf[TX_BUF_SZ];

int main(void)
{
  STD_PROLOG_UART_NOCON;

  xTaskCreate( task_leds, "leds", 1*def_stksz, 0, 1, 0 );
  xTaskCreate( task_send, "send", 2*def_stksz, 0, 1, 0 );

  SCHEDULER_START;
  return 0;
}


void task_send( void *prm UNUSED_ARG )
{
  strcpy( tx_buf, "ABCDE <.> 0123\r\n" );
  //               0123456789ABCDEF1011
  int ssz = strlen( tx_buf );
  char c = '?';

  while( 1 )  {
    // leds.toggle( BIT2 );
    if( HAL_UART_Receive( &uah, (uint8_t*)&c, 1, 0 ) == HAL_OK ) {
      leds.toggle( BIT2 );
      tx_buf[7] = c;
    }
    if( HAL_UART_Transmit( &uah, (uint8_t*)tx_buf, ssz, 100 )!= HAL_OK ) {
      // leds.toggle( BIT3 );
    }
    delay_ms( 1000 );
  }
}



// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

