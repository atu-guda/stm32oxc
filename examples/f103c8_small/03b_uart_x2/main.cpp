#include <cstring>

#include <oxc_auto.h>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

UART_HandleTypeDef uah;
int out_uart( const char *d, unsigned n );
void UART_handleIRQ();

int out_uart( const char *d, unsigned n )
{
  return HAL_UART_Transmit( &uah, (uint8_t*)d, n, 10 ) == HAL_OK;
}

void USART1_IRQHandler(void) {
  leds.toggle( BIT3 ); // DEBUG
  UART_handleIRQ();
}

volatile char in_char = ' ';

void UART_handleIRQ()
{
  uint32_t status = USART1->USART_SR_REG;
  bool on_transmit = false;

  // leds.toggle( BIT3 ); // DEBUG

  if( status & UART_FLAG_RXNE ) { // char recived
    leds.set( BIT0 );
    // ++n_work;
    // char cr = recvRaw();
    in_char = USART1->USART_RX_REG;
    // leds.set( BIT2 );
    if( status & ( UART_FLAG_ORE | UART_FLAG_FE /*| UART_FLAG_LBD*/ ) ) { // TODO: on MCU
      // err = status;
    } else {
      // charsFromIrq( &cr,  1 );
    }
    // leds.reset( BIT2 );
  }

  // TXE is keeps on after transmit
  if( on_transmit  &&  (status & UART_FLAG_TXE) ) {
    // leds.set( BIT0 );
    // ++n_work;
    // qrec = obuf.recvFromISR( &cs, &wake );
    // if( qrec == pdTRUE ) {
    //  sendRaw( cs );
    // } else {
    //  itDisable( UART_IT_TXE );
    //  on_transmit = false;
    //}
    // leds.reset( BIT0 );
  }


  // if( n_work == 0 ) { // unhandled
  //   // leds_toggle( BIT1 );
  // }

  //portEND_SWITCHING_ISR( wake );

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
  os << "CR1: " << HexInt16( USART1->CR1 )  << "  CR2: " << HexInt16( USART1->CR2 )  << "\r\n";
  os.flush();

  int n = 0;
  while( 1 ) {
    bool was_action = false;
    char c = '?', c_err = '-';
    os.clear();

    if( __HAL_USART_GET_FLAG( &uah, UART_FLAG_ORE ) ) { // overrun
      c = uah.Instance->USART_RX_REG;
      __HAL_USART_CLEAR_OREFLAG( &uah );
      was_action = true;
      c_err = 'O';
      // leds.toggle( BIT0 );
    }

    if( __HAL_USART_GET_FLAG( &uah, UART_FLAG_RXNE ) ) {
      c = uah.Instance->USART_RX_REG;
      was_action = true;
      // leds.toggle( BIT2 );
    }

    if( was_action ) {
      leds.toggle( BIT1 );
      os << "A:Z <";
      os << ( int8_t(c) >= ' '  ?  c : ' ' );
      os << "> [" << HexInt8( c ) << "] R" << c_err << cn << " <"
         << in_char << "> " << HexInt16( USART1->USART_SR_REG ) << " -\r\n";
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

