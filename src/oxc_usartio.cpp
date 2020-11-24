#include <oxc_usartio.h>

int UsartIO::write_s( const char *s, int l )
{
  if( !s  ||  l < 1 ) {
    return 0;
  }

  int ns = 0, nl = 0;
  for( int i=0; i<l; ++i,++s ) {
    // leds.toggle( BIT3 ); // DEBUG
    while( checkFlag( UART_FLAG_TXE ) == RESET ) {
      delay_mcs( 100 );
      taskYieldFun();
      ++nl;
      if( nl >= wait_tx ) {
        return ns;
      }
    }
    sendRaw( *s );
    ++ns;
  }
  return ns;
}

Chst UsartIO::getc_p( int w_tick )
{
  for( int i=0; i<w_tick || w_tick == 0; ++i ) { // w_tick == 0 means forever
    if( checkFlag( UART_FLAG_RXNE ) ) {
      char c = recvRaw();
      return c;
    }
    taskYieldFun();
  }

  return Chst( '\0', Chst::st_full );
}


void UsartIO::handleIRQ()
{
  int n_work = 0;
  uint32_t status = us->USART_SR_REG;

  // leds.set( BIT3 ); // DEBUG

  if( status & UART_FLAG_RXNE ) { // char received
    // leds.set( BIT2 );
    ++n_work;
    char cr = recvRaw();
    charFromIrq( cr );
    // leds.set( BIT2 );
    if( status & ( UART_FLAG_ORE | UART_FLAG_FE /*| UART_FLAG_LBD*/ ) ) { // TODO: on MCU
      err = status;
    }
    // leds.reset( BIT2 );
  }

  // TXE is keeps on after transmit
  if( on_transmit  &&  (status & UART_FLAG_TXE) ) {
    // leds.set( BIT0 );
    ++n_work;
    auto v = obuf.tryGet();
    if( v.good() ) {
      sendRaw( v.c );
    } else {
      itDisable( UART_IT_TXE );
      on_transmit = false;
    }
    // leds.reset( BIT0 );
  }


  if( n_work == 0 ) { // unhandled
    // leds_toggle( BIT1 );
  }

  wakeFromIRQ( 1 );
}

void UsartIO::start_transmit()
{
  if( on_transmit ) { // handle by IRQ
    return;
  }

  auto v = obuf.tryGet();
  if( v.good() ) {
    on_transmit = true;
    sendRaw( v.c );
    itEnable( UART_IT_TXE );
  }
}

void UsartIO::on_tick_action_tx()
{
  start_transmit();
}


