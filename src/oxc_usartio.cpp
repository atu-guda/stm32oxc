#include <oxc_usartio.h>
// #include <oxc_gpio.h> // debug

int UsartIO::sendBlockSync( const char *s, int l )
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

int UsartIO::recvBytePoll( char *b, int w_tick )
{
  if( !b ) { return 0; }

  for( int i=0; i<w_tick || w_tick == 0; ++i ) { // w_tick == 0 means forever
    if( checkFlag( UART_FLAG_RXNE ) ) {
      *b = recvRaw();
      return 1;
    }
    taskYieldFun();
  }

  return 0;
}


void UsartIO::handleIRQ()
{
  char cr, cs;
  int n_work = 0;
  BaseType_t wake = pdFALSE;
  BaseType_t qrec;
  uint32_t status = us->USART_SR_REG;

  // leds.set( BIT3 ); // DEBUG

  if( status & UART_FLAG_RXNE ) { // char recived
    // leds.set( BIT2 );
    ++n_work;
    cr = recvRaw();
    // leds.set( BIT2 );
    if( status & ( UART_FLAG_ORE | UART_FLAG_FE /*| UART_FLAG_LBD*/ ) ) { // TODO: on MCU
      err = status;
    } else {
      xQueueSendFromISR( ibuf, &cr, &wake  );
    }
    // leds.reset( BIT2 );
  }

  // TXE is keeps on after transmit
  if( on_transmit  &&  (status & UART_FLAG_TXE) ) {
    // leds.set( BIT0 );
    ++n_work;
    qrec = xQueueReceiveFromISR( obuf, &cs, &wake );
    if( qrec == pdTRUE ) {
      sendRaw( cs );
    } else {
      itDisable( UART_IT_TXE );
      on_transmit = false;
    }
    // leds.reset( BIT0 );
  }


  if( n_work == 0 ) { // unhandled
    // leds_toggle( BIT1 );
  }

  portEND_SWITCHING_ISR( wake );

}

void UsartIO::task_send()
{
  // leds.set( BIT0 );
  if( on_transmit ) { // handle by IRQ
    // leds.reset( BIT0 );
    return;
  }

  char ct;
  BaseType_t ts = xQueueReceive( obuf, &ct, wait_tx );
  if( ts == pdTRUE ) {
    on_transmit = true;
    // leds.toggle( BIT1 ); // DEBUG
    sendRaw( ct );
    itEnable( UART_IT_TXE );
  }
  // leds.reset( BIT0 );

}


