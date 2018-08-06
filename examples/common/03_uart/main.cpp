#include <cstring>

#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

extern "C" {
void task_leds( void *prm UNUSED_ARG );
void task_send( void *prm UNUSED_ARG );
} // extern "C"

UART_HandleTypeDef uah_console;

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
  char c = '?', cn = '0';
  static const char ostr[] = "A:Z <.> [  ] -----\r\n";
  //                          0123456789ABCDEF1011
  static const auto sz_oszr = strlen( ostr );
  const unsigned c_pos = 5, hex_pos = 9, read_pos = 13, ovr_pos = 14, i_pos = 16;

  int n = 0;
  while( 1 ) {
    bool was_action = false;
    strcpy( tx_buf, ostr );

    tx_buf[i_pos] = cn++;
    if( cn >= 0x7F ) { cn = ' '; }

    if( __HAL_USART_GET_FLAG( &uah_console, UART_FLAG_ORE ) ) { // overrun
      c = uah_console.Instance->USART_RX_REG;
      __HAL_USART_CLEAR_OREFLAG( &uah_console );
      was_action = true;
      tx_buf[ovr_pos] = 'O';
      leds.toggle( BIT1 );
    }

    if( __HAL_USART_GET_FLAG( &uah_console, UART_FLAG_RXNE ) ) {
      c = uah_console.Instance->USART_RX_REG;
      was_action = true;
      leds.toggle( BIT2 );
    }

    if( was_action ) {
      char2hex( c, tx_buf + hex_pos ); tx_buf[hex_pos+2] = ']';
      if( int8_t(c) >= ' ' ) {
        tx_buf[c_pos]  = c;
      }
      tx_buf[read_pos] = 'R';
      if( HAL_UART_Transmit( &uah_console, (uint8_t*)tx_buf, sz_oszr, 10 ) != HAL_OK ) {
        leds.toggle( BIT0 );
      } else {
        leds.toggle( BIT3 );
      }
    } else if( n % ( 20*delay_calibrate_value) == 0 ) {
      HAL_UART_Transmit( &uah_console, (uint8_t*)("-\r\n"), 3, 10 );
    } else {
      // NOP;
    }

    ++n;
  }
}



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

