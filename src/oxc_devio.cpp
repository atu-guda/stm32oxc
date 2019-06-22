#include <cstring>
#include <oxc_devio.h>

using namespace std;

DevIO* devio_fds[DEVIO_MAX];     // for fd-based access, may be dupes or empty at all
DevIO* DevIO::devios[DEVIO_MAX]; // for inner array action

DevIO::DevIO( unsigned ibuf_sz, unsigned obuf_sz  )
     : ibuf( ibuf_sz ),
       obuf( obuf_sz )
{
  // register
  for( int i=0; i< DEVIO_MAX; ++i ) {
    if( devios[i] == nullptr ) {
      devios[i] = this;
      break;
    }
  }
}

DevIO::~DevIO()
{
  reset();
  // unregister
  for( int i=0; i< DEVIO_MAX; ++i ) {
    if( devios[i] == this ) {
      devios[i] = nullptr;
      break;
    }
  }
}

void DevIO::reset()
{
  ibuf.reset();
  obuf.reset();
}

int DevIO::write( const char *s, int l )
{
  if( !s ) {
    return -1;
  }
  if( l < 1 ) {
    return 0;
  }

  int ns = 0;
  if( ( ns = obuf.puts_ato( s, l ) ) <= 0 ) {
    ns = obuf.puts( s, l );
  }

  // start_transmit();

  if( ns >0 ) {
    taskYieldFun();
  }

  return ns;
}


Chst DevIO::getc( int w_tick )
{
  for( int i=0; i < w_tick || w_tick == 0; ++i ) { // w_tick == 0 means forever
    auto v = ibuf.tryGet();
    if( v.good() ) {
      return v;
    }
  }

  return 0;
}

void DevIO::on_tick_action_tx()
{
  if( on_transmit ) { return; } // handle by IRQ?
  char tbuf[TX_BUF_SIZE];
  unsigned ns = obuf.tryGets( tbuf, sizeof(tbuf ) );
  if( ns > 0 ) {
    write_s( tbuf, ns ); // TODO: if( send_now )?
  }

}


void DevIO::on_tick_action_rx() // really a fallback, may be called from IRQ!
{
  if( onRecv != nullptr ) {
    auto v = ibuf.tryGet();
    if( v.good() ) {
      onRecv( &v.c, 1 );
    }
  }
}


void DevIO::on_tick_action() // really a fallback, may be called from IRQ!
{
  on_tick_action_rx();
  on_tick_action_tx();
}

void DevIO::tick_actions_all()
{
  for( int i=0; i< DEVIO_MAX; ++i ) {
    if( devios[i] != nullptr ) {
      devios[i]->on_tick_action();
    }
  }
}

void DevIO::testCbreak( char c )
{
  if( handle_cbreak && c == 3 ) {
      break_flag = 1;
    if( onSigInt ) {
      onSigInt( c );
    }
  }
}

void DevIO::charFromIrq( char c ) // called from IRQ!
{
  testCbreak( c );
  ibuf.tryPut( c );
  wakeFromIRQ( 1 );
}

void DevIO::charsFromIrq( const char *s, int l ) // called from IRQ!
{
  for( int i=0; i<l; ++i ) {
    testCbreak( s[i] );
    ibuf.tryPut( s[i] );
  }
  wakeFromIRQ( 1 );
}

int DevIO::wait_eot( int w )
{
  for( auto i = 0; i < w; ++i ) {
    if( ! on_transmit && obuf.size() == 0 ) {
      return 1;
    }
    delay_ms( 1 );
  }
  return 0;
}

int DevIO::puts( const char *s )
{
  if( !s ) { return 0; }
  return write( s, strlen(s) );
}

int DevIO::puts_s( const char *s )
{
  if( !s ) { return 0; }
  return write_s( s, strlen(s) );
}


int DevIO::read( char *s, int l, int w_tick )
{
  if( !s ) { return 0; }
  int n;
  for( n=0; n<l; ++n,++s ) {
    Chst k = getc( w_tick );
    if( ! k.good() ) {
      return n;
    }
  }
  return n;
}

int DevIO::read_poll( char *s, int l, int w_tick )
{
  if( !s ) { return 0; }
  int n;
  for( n=0; n<l; ++n,++s ) {
    auto k = getc_p( w_tick );
    if( ! k.good() ) {
      return n;
    }
    *s = k.c;
  }
  return n;
}

#define COMMON_FD_TEST(fd) \
  if( fd < 0 || fd > DEVIO_MAX || !devio_fds[fd] ) {\
    return 0; \
  }

Chst getChar( int fd, int w_tick )
{
  COMMON_FD_TEST( fd );
  return devio_fds[fd]->getc( w_tick );
}

Chst tryGet( int fd )
{
  COMMON_FD_TEST( fd );
  return devio_fds[fd]->tryGet();
}

unsigned tryGetLine( int fd, char *d, unsigned max_len )
{
  COMMON_FD_TEST( fd );
  return devio_fds[fd]->tryGetLine( d, max_len );
}

int unget( int fd, char c )
{
  COMMON_FD_TEST( fd );
  devio_fds[fd]->unget( c );
  return 1;
}

int ungets( int fd, const char *s )
{
  COMMON_FD_TEST( fd );
  for( ; *s; ++s ) {
    devio_fds[fd]->unget( *s );
  }
  return 1;
}

int reset_in( int fd )
{
  COMMON_FD_TEST( fd );
  devio_fds[fd]->reset_in();
  return 1;
}

int reset_out( int fd )
{
  COMMON_FD_TEST( fd );
  devio_fds[fd]->reset_out();
  return 1;
}

int reset_io( int fd )
{
  COMMON_FD_TEST( fd );
  devio_fds[fd]->reset();
  return 1;
}

int write( int fd, const char *s, int l )
{
  COMMON_FD_TEST( fd );
  return devio_fds[fd]->write( s, l );
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
  write( fd, s, l );
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

