#include <cstring>


#include <oxc_auto.h>

#include <utility>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

UART_HandleTypeDef uah;
int out_uart( const char *d, unsigned n );
void UART_handleIRQ();

// -------------------------------------------------------------------


// int out_uart( const char *d, unsigned n )
// {
//   return HAL_UART_Transmit( &uah, (uint8_t*)d, n, 10 ) == HAL_OK;
// }

const unsigned buf_sz = 32;

char ring_tx_buf[buf_sz];
RingBuf tx_ring( ring_tx_buf, sizeof( ring_tx_buf ) );
char ring_rx_buf[buf_sz];
RingBuf rx_ring( ring_rx_buf, sizeof( ring_rx_buf ) );

void BOARD_UART_DEFAULT_IRQHANDLER(void) {
  // leds.toggle( BIT3 ); // DEBUG
  UART_handleIRQ();
  HAL_UART_IRQHandler( &uah );
}

volatile char in_char = ' ';
bool on_transmit = false;

void UART_handleIRQ()
{
  uint16_t status = BOARD_UART_DEFAULT->USART_SR_REG;

  // leds.toggle( BIT0 ); // DEBUG

  if( status & UART_FLAG_RXNE ) { // char recived
    leds.toggle( BIT2 );
    // ++n_work;
    // char cr = recvRaw();
    in_char = BOARD_UART_DEFAULT->USART_RX_REG;
    if( status & ( UART_FLAG_ORE | UART_FLAG_FE /*| UART_FLAG_LBD*/ ) ) { // TODO: on MCU
      // err = status;
    } else {
      if( ! rx_ring.tryPut( in_char ) ) {
         // leds.toggle( BIT0 );
      }
    }
    // leds.reset( BIT2 );
  }

  // if( status & UART_FLAG_TXE ) {
  //   leds.toggle( BIT1 );
  // }
  if( on_transmit  && ( status & UART_FLAG_TXE ) ) {
    // ++n_work;
    leds.toggle( BIT1 );
    auto toOut = tx_ring.tryGet();
    if( toOut.good() ) {
     // sendRaw( cs );
      BOARD_UART_DEFAULT->USART_TX_REG = toOut.c;
    } else {
      BOARD_UART_DEFAULT->CR1 &= ~USART_CR1_TXEIE;
      // itDisable( UART_IT_TXE );
      on_transmit = false;
    }
  }


  // if( n_work == 0 ) { // unhandled
  //   // leds_toggle( BIT1 );
  // }

  //portEND_SWITCHING_ISR( wake );

}




int main(void)
{
  STD_PROLOG_UART_NOCON;

  leds.write( 0 );

  // uint32_t c_msp = __get_MSP();
  // os << " MSP-__heap_top = " << ((unsigned)c_msp - (unsigned)(__heap_top) ) << "\r\n";
  // os.flush();
  // os << "CR1: " << HexInt16( BOARD_UART_DEFAULT->CR1 )  << "  CR2: " << HexInt16( BOARD_UART_DEFAULT->CR2 )  << "\r\n";
  // os.flush();

  int n = 0;

  BOARD_UART_DEFAULT->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;

  while( 1 ) {
    // leds.toggle( BIT3 );

    // if( rx_ring.isFull() ) {
    //   // BOARD_UART_DEFAULT->USART_TX_REG = 'F';
    //   delay_ms( 1 );
    // }
    //
    // --------------------------- IO logic loop here

    auto rec = rx_ring.get();

    if( rec.good() ) {
      // leds.toggle( BIT2 );
      // BOARD_UART_DEFAULT->USART_TX_REG = rec.c;
      // delay_ms( 5 );
    } else if( rec.locked() ) {
      // BOARD_UART_DEFAULT->USART_TX_REG = 'L';
      // delay_ms( 5 );
    } else {
      // leds.toggle( BIT3 );
      delay_ms( 10 );
      // BOARD_UART_DEFAULT->USART_TX_REG = char( '0' + rec.st );
      // BOARD_UART_DEFAULT->USART_TX_REG = 'L';
    }

    if( !on_transmit ) {
      auto toOut = tx_ring.tryGet();
      if( toOut.good() ) {
        on_transmit = true;
        BOARD_UART_DEFAULT->USART_TX_REG = toOut.c;
        BOARD_UART_DEFAULT->CR1 |= USART_CR1_TXEIE;
      }
    }
    // --------------------------- IO logic loop end

    if( n % ( 100 /* delay_calibrate_value */ ) == 0 ) {
      leds.toggle( BIT3 );
      // BOARD_UART_DEFAULT->USART_TX_REG = '+';
      tx_ring.tryPuts( "ABCD" NL );
      delay_ms( 10 );
    }

    ++n;
  }


  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

