#include <cstring>
#include <oxc_devio.h>

using namespace std;

DevIO* devio_fds[DEVIO_MAX];
volatile int idle_flag = 0;
volatile int break_flag = 0;

DevIO::~DevIO()
{
  reset();
}

void DevIO::reset()
{
  ibuf.reset();
  obuf.reset();
}

int DevIO::sendBlock( const char *s, int l )
{
  if( !s  ||  l < 1 ) {
    return 0;
  }

  int ns = 0, sst;

  for( int i=0; i<l; ++i ) {
    sst = obuf.put( s[i] );
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

  auto r = ibuf.get();
  if( r.good() ) {
    *b = r.c;
    return 1;
  }

  return 0;
}

int DevIO::task_send()
{
  // if( on_transmit ) { return; } // handle by IRQ
  int ns = 0;
  int wait_now = wait_tx;
  char ct;
  for( ns=0; ns<TX_BUF_SIZE; ++ns ) {
    BaseType_t ts = obuf.recv( &ct, wait_now );
    if( ts != pdTRUE ) { break; };
    tx_buf[ns] = ct;
    wait_now = 0;
  }
  if( ns == 0 ) { return; }
  return sendBlockSync( tx_buf, ns ); // TODO: if( send_now )?
};

int DevIO::task_recv()
{
  // leds.set( BIT2 );
  auto ts = ibuf.tryGet();
  if( ts.good() ) {
    if( onRecv != nullptr ) {
      onRecv( &ts.c, 1 );
    } else {
      // else simply eat char - if not required - dont use this task
    }
  }
  // leds.reset( BIT2 );
}

int DevIO::task_io()
{
  int n = task_recv();
  n += task_send();
  return n;
}

void DevIO::charFromIrq( char c ) // called from IRQ!
{
  BaseType_t wake = pdFALSE;
  if( c == 3  && onSigInt ) { // handle Ctrl-C = 3
    onSigInt( c );
  }
  ibuf.tryPut( c );
  portEND_SWITCHING_ISR( wake );
}

void DevIO::charsFromIrq( const char *s, int l ) // called from IRQ!
{
  BaseType_t wake = pdFALSE;
  for( int i=0; i<l; ++i ) {
    if( s[i] == 3  && onSigInt ) { // handle Ctrl-C = 3
      onSigInt( s[i] );
    }
    ibuf.tryPut( s[i]  );
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

#define COMMON_FD_TEST(fd) \
  if( fd < 0 || fd > DEVIO_MAX || !devio_fds[fd] ) {\
    return 0; \
  }

int recvByte( int fd, char *s, int w_tick )
{
  COMMON_FD_TEST( fd );
  return devio_fds[fd]->recvByte( s, w_tick );
}

int sendBlock( int fd, const char *s, int l )
{
  COMMON_FD_TEST( fd );
  return devio_fds[fd]->sendBlock( s, l );
}

int pr( const char *s, int fd /* = 1 */ )
{
  if( !s || !*s ) {
    return 0;
  }
  prl( s, strlen(s), fd );
  return 0;
}

int prl( const char *s, unsigned l, int fd /* = 1 */  )
{
  sendBlock( fd, s, l );
  idle_flag = 1;
  return 0;
}

int prl1( const char *s, unsigned l  )
{
  return prl( s, l, 1 );
}


// void DevIO::initIRQ( uint8_t ch, uint8_t prio )
// {
//   HAL_NVIC_SetPriority( ch, prio, 0 );
// }

