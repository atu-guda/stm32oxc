#include <cstring>
#include <oxc_devio.h>

using namespace std;

DevIO::~DevIO()
{
  reset();
  vQueueDelete( obuf );
  vQueueDelete( ibuf );
}

void DevIO::reset()
{
  xQueueReset( ibuf );
  xQueueReset( obuf );
}

int DevIO::sendBlock( const char *s, int l )
{
  if( !s  ||  l < 1 ) {
    return 0;
  }

  int ns = 0, sst;

  for( int i=0; i<l; ++i ) {
    sst = xQueueSend( obuf, &(s[i]), wait_tx );
    if( sst != pdTRUE ) {
      err = 10;
      break;
    }
    ++ns;
  }
  if( ns ) {
    taskYieldFun();
  }

  return ns;
}


int DevIO::recvByte( char *b, int w_tick )
{
  if( !b ) { return 0; }

  char c;
  BaseType_t r = xQueueReceive( ibuf, &c, w_tick );
  if( r == pdTRUE ) {
    *b = c;
    return 1;
  }

  return 0;
}

void DevIO::task_send()
{
  // if( on_transmit ) { return; } // handle by IRQ
  const int xbufsz = 32;
  char xbuf[xbufsz]; // TODO: param
  int ns = 0;
  int wait_now = wait_tx;
  char ct;
  for( ns=0; ns<xbufsz; ++ns ) {
    BaseType_t ts = xQueueReceive( obuf, &ct, wait_now );
    if( ts != pdTRUE ) { break; };
    xbuf[ns] = ct;
    wait_now = 0;
  }
  if( ns == 0 ) { return; }
  sendBlockSync( xbuf, ns );
};

void DevIO::task_recv()
{
  // leds.set( BIT2 );
  char cr;
  BaseType_t ts = xQueueReceive( ibuf, &cr, wait_rx );
  if( ts == pdTRUE ) {
    if( onRecv != nullptr ) {
      onRecv( &cr, 1 );
    } else {
      // else simply eat char - if not required - dont use this task
    }
  }
  // leds.reset( BIT2 );
}

void DevIO::charsFromIrq( const char *s, int l )
{
  BaseType_t wake = pdFALSE;
  for( int i=0; i<l; ++i ) {
    xQueueSendFromISR( ibuf, s+i, &wake  );
  }
  portEND_SWITCHING_ISR( wake );
}

int DevIO::sendStr( const char *s )
{
  if( !s ) { return 0; }
  return sendBlock( s, strlen(s) );
}

int DevIO::sendStrSync( const char *s )
{
  if( !s ) { return 0; }
  return sendBlockSync( s, strlen(s) );
}


int DevIO::recvBlock( char *s, int l, int w_tick )
{
  if( !s ) { return 0; }
  int n;
  for( n=0; n<l; ++n,++s ) {
    int k = recvByte( s, w_tick );
    if( k < 1 ) {
      return n;
    }
  }
  return n;
}

int DevIO::recvBlockPoll( char *s, int l, int w_tick )
{
  if( !s ) { return 0; }
  int n;
  for( n=0; n<l; ++n,++s ) {
    int k = recvBytePoll( s, w_tick );
    if( k < 1 ) {
      return n;
    }
  }
  return n;
}

// void DevIO::initIRQ( uint8_t ch, uint8_t prio )
// {
//   HAL_NVIC_SetPriority( ch, prio, 0 );
// }

