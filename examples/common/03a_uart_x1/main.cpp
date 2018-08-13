#include <cstring>

#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

UART_HandleTypeDef uah_console;
int out_uart( const char *d, unsigned n );

int out_uart( const char *d, unsigned n )
{
  return HAL_UART_Transmit( &uah_console, (uint8_t*)d, n, 10 ) == HAL_OK;
}



int main(void)
{
  STD_PROLOG_UART_NOCON;

  char cn = '0';
  leds.write( 0 );
  MSTRF( os, 128, out_uart );

  uint32_t c_msp = __get_MSP();
  os << " MSP-__heap_top = " << ((unsigned)c_msp - (unsigned)(__heap_top) ) << "\r\n";
  os.flush();

  int n = 0;
  while( 1 ) {
    bool was_action = false;
    char c = '?', c_err = '-';
    os.clear();

    if( __HAL_USART_GET_FLAG( &uah_console, UART_FLAG_ORE ) ) { // overrun
      c = uah_console.Instance->USART_RX_REG;
      __HAL_USART_CLEAR_OREFLAG( &uah_console );
      was_action = true;
      c_err = 'O';
      leds.toggle( BIT0 );
    }

    if( __HAL_USART_GET_FLAG( &uah_console, UART_FLAG_RXNE ) ) {
      c = uah_console.Instance->USART_RX_REG;
      was_action = true;
      leds.toggle( BIT2 );
    }

    if( was_action ) {
      leds.toggle( BIT1 );
      os << "A:Z <";
      os << ( int8_t(c) >= ' '  ?  c : ' ' );
      os << "> [" << HexInt8( c ) << "] R" << c_err << cn << "-\r\n";
      os.flush();
    } else if( n % ( 50*delay_calibrate_value ) == 0 ) {
      leds.toggle( BIT1 );
      os << '-' << cn << "-\r\n";
      os.flush();
    } else {
      // NOP;
    }

    ++cn;
    if( cn >= 0x7F ) { cn = ' '; }

    ++n;
  }


  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

