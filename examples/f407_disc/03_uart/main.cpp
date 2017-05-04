#include <cstring>

#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
FreeRTOS_to_stm32cube_tick_hook;
BOARD_DEFINE_LEDS;

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
  strcpy( tx_buf, "ABCDE <.> -----\r\n" );
  //               0123456789ABCDEF1011
  int ssz = strlen( tx_buf );
  char c = '?', cn = '0';

  while( 1 ) {
    leds.toggle( BIT1 );
    tx_buf[5]  = ' ';
    tx_buf[10] = cn; tx_buf[11] = '.';
    ++cn;
    if( cn >= 0x7F ) { cn = ' '; }
    // if( HAL_UART_Receive( &uah, (uint8_t*)&c, 1, 0 ) == HAL_OK ) {
    //   leds.toggle( BIT2 );
    //   tx_buf[5]  = c; tx_buf[11] = 'R';
    // }
    if( uah.Instance->USART_SR_REG & UART_FLAG_RXNE ) {
      c = uah.Instance->USART_RX_REG;
      leds.toggle( BIT2 );
      tx_buf[7]  = c; tx_buf[11] = 'R';
    }
    if( uah.Instance->USART_SR_REG & UART_FLAG_ORE ) { // overrun
      #ifdef UART_CLEAR_OREF
        uah.Instance->ICR |= UART_CLEAR_OREF;
      #endif
      c = uah.Instance->USART_RX_REG;
      tx_buf[12] = 'O';
      leds.toggle( BIT0 );
    }
    if( HAL_UART_Transmit( &uah, (uint8_t*)tx_buf, ssz, 100 ) != HAL_OK ) {
      // leds.toggle( BIT0 );
    }
    delay_ms( 1000 );
  }
}



// vim: path=.,/usr/share/stm32lib/inc/,/usr/arm-none-eabi/include,../../../inc

