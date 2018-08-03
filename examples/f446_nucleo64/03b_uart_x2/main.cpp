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
}

volatile bool on_transmit = false;
int  start_transmit();
void wait_eot();

void UART_handleIRQ()
{
  uint16_t status = BOARD_UART_DEFAULT->USART_SR_REG;

  leds.set( BIT3 ); // DEBUG

  if( status & UART_FLAG_RXNE ) { // char recived
    // leds.toggle( BIT2 );
    // ++n_work;
    // char cr = recvRaw();
    char in_char = BOARD_UART_DEFAULT->USART_RX_REG; // TODO: link to given object
    if( status & ( UART_FLAG_ORE | UART_FLAG_FE /*| UART_FLAG_LBD*/ ) ) { // TODO: on MCU
      // err = status;
    } else {
      if( ! rx_ring.tryPut( in_char ) ) {
         // leds.toggle( BIT0 );
      } // TODO: else
    }
    // leds.reset( BIT2 );
  }

  if( on_transmit  && ( status & UART_FLAG_TXE ) ) {
    // ++n_work;
    leds.set( BIT1 );
    // auto toOut = tx_ring.getFromISR();
    auto toOut = tx_ring.tryGet();
    if( toOut.good() ) { // TODO: all cases
     // sendRaw( cs );
      BOARD_UART_DEFAULT->USART_TX_REG = toOut.c;
    } else if ( toOut.empty() ) {
      BOARD_UART_DEFAULT->CR1 &= ~USART_CR1_TXEIE;
      // itDisable( UART_IT_TXE );
      on_transmit = false;
      leds.reset( BIT0 );
    } else { // locked, full?
      // BOARD_UART_DEFAULT->USART_TX_REG = '@';
      BOARD_UART_DEFAULT->CR1 &= ~USART_CR1_TXEIE;
      // itDisable( UART_IT_TXE );
      on_transmit = false;
      leds.reset( BIT0 );
    }
    leds.reset( BIT1 );
  }


  // if( n_work == 0 ) { // unhandled
  //   // leds_toggle( BIT1 );
  // }

  //portEND_SWITCHING_ISR( wake );
  leds.reset( BIT3 ); // DEBUG

}


void out( const char *s )
{
  if( !s || !*s ) {
    return;
  }
  if( on_transmit ) {
    tx_ring.puts( s );
    if( on_transmit ) { // TODO: better: restart
      return;
    }
  }

  // start transmit
  // char c0 = *s;
  // ++s;
  if( tx_ring.puts_ato( s ) > 0 ) {
    start_transmit();
    // BOARD_UART_DEFAULT->USART_TX_REG = c0;
    // on_transmit = true;
    // BOARD_UART_DEFAULT->CR1 |= USART_CR1_TXEIE;
    // leds.set( BIT0 );
    return;
  }

  leds.set( BIT2 );
  wait_eot();
  tx_ring.tryPut( *s++ );
  tx_ring.tryPut( *s++ );
  start_transmit();
  tx_ring.puts( s );
  leds.reset( BIT2 );
}

int start_transmit()
{
  for( ; ; ) {
    auto v = tx_ring.tryGet();
    if( v.empty() ) {
      return 0;
    }
    if( v.locked() ) {
      delay_mcs(1);
      continue;
    }
    if( v.good() ) {
      BOARD_UART_DEFAULT->USART_TX_REG = v.c;
      on_transmit = true;
      BOARD_UART_DEFAULT->CR1 |= USART_CR1_TXEIE;
      leds.set( BIT0 );
      return 1;
    }
  };
}

void wait_eot()
{
  while( on_transmit ) {
    delay_ms( 1 );
  }
}

char huge_str[128];


int main(void)
{
  STD_PROLOG_UART_NOCON;

  leds.write( 0 );

  int n = 0;
  char ou[12];
  // strcpy( ou, "ABC_0[z] !!!" NL );
  strcpy( ou, "ABC_0[?]"  );
  strcpy( huge_str, "0123456789-ABCDEFGHIJKLMNOPQRTUVWXYZ-abcdefghijklmnopqrtuvwxyz>" NL );
  // memset( huge_str, '$', sizeof(huge_str) ); huge_str[sizeof(huge_str)-1] = '\0';

  tx_ring.set_n_wait( 1000 );


  BOARD_UART_DEFAULT->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;

  while( 1 ) {

    // leds.toggle( BIT3 );

    auto rec = rx_ring.get();

    ++ou[4];
    if( ou[4] > 'Z' ) { ou[4] = '0'; }
    ou[6] = rec.good() ?  rec.c : ' ';
    if( ou[4] == 'L' ) {
      out( huge_str );
    } else {
      out( ou );
      // wait_eot();
      out( " !!!" NL );
      // out( " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!" NL );
    }
    delay_ms( 500 );

    ++n;
  }


  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

