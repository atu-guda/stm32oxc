#include <cstring>

#include <oxc_auto.h>
#include <oxc_main.h>

#include <utility>

using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

UART_HandleTypeDef uah_console;
int out_uart( const char *d, unsigned n );
void UART_handleIRQ();

// -------------------------------------------------------------------


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
void start_transmit();
void wait_eot();


void UART_handleIRQ()
{
  uint16_t status = BOARD_UART_DEFAULT->USART_SR_REG;

  // leds.set( BIT3 ); // DEBUG

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
    } else  {
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
  // leds.reset( BIT3 ); // DEBUG

}


void out( const char *s )
{
  if( !s || !*s ) {
    return;
  }

  if( tx_ring.puts_ato( s ) > 0 ) {
    start_transmit();
    return;
  }

  leds.set( BIT2 );
  tx_ring.tryPut( *s++ );
  start_transmit();
  tx_ring.puts( s );
  leds.reset( BIT2 );
}

void start_transmit()
{
  if( on_transmit ) {
    return;
  }

  auto v = tx_ring.tryGet();
  if( v.good() ) {
    BOARD_UART_DEFAULT->USART_TX_REG = v.c;
    on_transmit = true;
    BOARD_UART_DEFAULT->CR1 |= USART_CR1_TXEIE;
    leds.set( BIT0 );
  }
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
  strcpy( ou, "ABC_a[?]"  );
  strcpy( huge_str, "0123456789-ABCDEFGHIJKLMNOPQRTUVWXYZ-abcdefghijklmnopqrtuvwxyz>" NL );

  oxc_add_aux_tick_fun( start_transmit );

  tx_ring.set_n_wait( 1000 );


  BOARD_UART_DEFAULT->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;

  while( 1 ) {

    // leds.toggle( BIT3 );

    auto rec = rx_ring.get();

    ++ou[4];
    if( ou[4] > 'z' ) { ou[4] = 'a'; }
    ou[6] = rec.good() ?  rec.c : ' ';
    if( ou[6] < ' ' ) {
      ou[6] = '\xAA';
    }
    if( ou[4] == 'l' ) {
      out( huge_str );
    } else {
      out( ou );
      out( " !!!" NL );
      /// out( " !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!." NL );
    }

    if( ! rec.good() ) {
      delay_ms( 500 );
    }

    ++n;
  }


  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

