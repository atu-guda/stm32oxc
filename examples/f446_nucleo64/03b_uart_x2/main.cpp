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

// TODO: classes for Mu_t handling: move to oxc_main or oxc_mu

#ifdef USE_OXC
using Mu_t  = mu_t;
#define Mu_lock(x)       mu_lock(x)
#define Mu_unlock(x)     mu_unlock(x)
#define Mu_trylock(x)    mu_trylock(x)
#define Mu_waitlock(x,m) mu_waitlock(x,m)
#define Mu_init          0
#else
#include <pthread.h>
using Mu_t  = pthread_mutex_t;
#define Mu_lock(x)       pthread_mutex_lock(x)
#define Mu_unlock(x)     pthread_mutex_unlock(x)
#define Mu_trylock(x)    pthread_mutex_trylock(x)
#define Mu_waitlock(x,m) pthread_mutex_waitlock(x,m)
#define Mu_init          PTHREAD_MUTEX_INITIALIZER
int pthread_mutex_waitlock( pthread_mutex_t *mutex, unsigned ms );
int pthread_mutex_waitlock( pthread_mutex_t *mutex, unsigned ms  )
{
  for( unsigned i=0; i<ms; ++i ) {
    if( pthread_mutex_trylock( mutex ) ) {
        return 1;
    }
  }
  return 0;
}

#endif

class MuLock {
  public:
   MuLock( Mu_t &a_mu ) : mu( a_mu ) { Mu_lock( &mu ); };
   ~MuLock()  { Mu_unlock( &mu ); };
  protected:
   Mu_t &mu;
};

class MuTryLock {
  public:
   MuTryLock( Mu_t &a_mu ) : mu( a_mu ), acq( !Mu_trylock( &mu ) ) { };
   ~MuTryLock()  { if( acq ) { Mu_unlock( &mu ); } };
   bool wasAcq() const { return acq; }
  protected:
   Mu_t &mu;
   const bool acq;
};

class MuWaitLock {
  public:
   MuWaitLock( Mu_t &a_mu, uint32_t ms = 100 ) : mu( a_mu ), acq( !Mu_waitlock( &mu, ms ) ) { };
   ~MuWaitLock()  { if( acq ) { Mu_unlock( &mu ); } };
   bool wasAcq() const { return acq; }
  protected:
   Mu_t &mu;
   const bool acq;
};

class Chst { // char + status
  public:
   enum {
     st_good = 0, st_full = 1, st_empty = 2, st_lock = 4
   };
   Chst( char ch ) : c( ch ), st( st_good ) {};
   Chst( char ch, uint8_t a_st ) : c( ch ), st( a_st ) {};
   char c;
   uint8_t st;
   bool good() const noexcept   { return st == st_good; }
   bool full() const noexcept   { return st == st_full; }
   bool empty() const noexcept  { return st == st_empty; }
   bool locked() const noexcept { return st == st_lock; }
};
static_assert( sizeof(Chst) == 2 );

class RingBuf {
  public:
   RingBuf( char *a_b, unsigned a_cap ); // used external buf
   // may be with dynamic buffer ?
   RingBuf( const RingBuf &r ) = delete;
   RingBuf& operator=( const RingBuf &rhs ) = delete;
   unsigned size() const { return sz; } // w/o block?
   unsigned capacity() const { return cap; }
   unsigned isFull() const { return sz == cap; }
   bool put( char c ); // blocks, wait
   bool tryPut( char c ); // noblocks, fail if busy
   bool waitPut( char c, uint32_t ms = 100 ); // wait + try, delay_ms(1)
   unsigned puts( const char *s ); // blocks, in one lock, ret: number of char
   unsigned puts( const char *s, unsigned l ); // blocks, given length
   unsigned tryPuts( const char *s ); // try-blocks
   unsigned tryPuts( const char *s, unsigned l ); // try-blocks, given length
   Chst get(); // blocks
   Chst tryGet(); // noblocks
   Chst waitGet( uint32_t ms = 100 ); // wait + try
   unsigned gets( char *d, unsigned max_len );
   unsigned tryGets( char *d, unsigned max_len );
   void reset() { MuLock lock( mu ); reset_nolock(); };
   void reset_nolock() { s = e = sz = 0; }
  protected:
   bool put_nolock( char c );
   Chst get_nolock();
   unsigned puts_nolock( const char *s );
   unsigned puts_nolock( const char *s, unsigned l );
   unsigned gets_nolock( char *d, unsigned max_len );
  protected:
   char *b;
   const unsigned cap;        //* capacity
   volatile unsigned sz = 0;  //* size
   volatile unsigned s  = 0;  //* start index (over the head)
   volatile unsigned e  = 0;  //* end index
   Mu_t mu = Mu_init;
};

RingBuf::RingBuf( char *a_b, unsigned a_cap )
  : b( a_b ), cap( a_cap )
{
}

bool RingBuf::put_nolock( char c )
{
  if( sz >= cap ) {
    return false;
  }
  unsigned sn = s + 1;
  if( sn >= cap ) {
    sn = 0;
  }
  b[s] = c; s = sn;  ++sz;
  return true;
}

bool RingBuf::put( char c )
{
  MuLock lock( mu );
  return put_nolock( c );
}

bool RingBuf::tryPut( char c )
{
  MuTryLock lock( mu );
  if( lock.wasAcq() ) {
    return put_nolock( c );
  }
  return false;
}

bool RingBuf::waitPut( char c, uint32_t ms  )
{
  MuWaitLock lock( mu, ms );
  if( lock.wasAcq() ) {
    return put_nolock( c );
  }
  return false;
}

unsigned RingBuf::puts_nolock( const char *s )
{
  unsigned r;
  for( r=0; *s && put_nolock( *s ) ; ++s, ++r ) {
  }
  return r;
}

unsigned RingBuf::puts_nolock( const char *s, unsigned l )
{
  unsigned r;
  for( r=0; r<l && put_nolock( *s ) ; ++s, ++r ) {
  }
  return r;
}


unsigned RingBuf::puts( const char *s )
{
  MuLock lock( mu );
  return puts_nolock( s );
}

unsigned RingBuf::puts( const char *s, unsigned l )
{
  MuLock lock( mu );
  return puts_nolock( s, l );
}

unsigned RingBuf::tryPuts( const char *s )
{
  MuTryLock lock( mu );
  if( ! lock.wasAcq() ) {
    return 0;
  }
  return puts_nolock( s );
}

unsigned RingBuf::tryPuts( const char *s, unsigned l )
{
  MuTryLock lock( mu );
  if( ! lock.wasAcq() ) {
    return 0;
  }
  return puts_nolock( s, l );
}


Chst RingBuf::get_nolock()
{
  if( sz < 1 ) {
    return Chst( '\0', Chst::st_empty );
  }
  char c = b[e++];
  if( e >= cap ) {
    e = 0;
  }
  --sz;
  return c;
}


Chst RingBuf::get()
{
  MuLock lock( mu );
  return get_nolock();
}

Chst RingBuf::tryGet()
{
  MuTryLock lock( mu );
  if( lock.wasAcq() ) {
    return get_nolock();
  }
  return Chst( '\0', Chst::st_lock );
}

Chst RingBuf::waitGet( uint32_t ms )
{
  MuWaitLock lock( mu, ms );
  if( lock.wasAcq() ) {
    return get_nolock();
  }
  return Chst( '\0', Chst::st_lock );
}

unsigned RingBuf::gets_nolock( char *d, unsigned max_len )
{
  if( !s  || max_len < 1 ) {
    return 0;
  }
  unsigned i;
  for( i=0; i<max_len; ++i ) {
    auto x = get_nolock();
    if( !x.good() ) {
      break;
    }
    *d++ = x.c;
  }
  return i;
}

unsigned RingBuf::gets( char *d, unsigned max_len )
{
  if( !s  || max_len < 1 ) {
    return 0;
  }
  MuLock lock( mu );
  return gets_nolock( d, max_len );
}

unsigned RingBuf::tryGets( char *d, unsigned max_len )
{
  if( !s  || max_len < 1 ) {
    return 0;
  }
  MuTryLock lock( mu );
  return gets_nolock( d, max_len );
}

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
      tx_ring.tryPuts( "ABCDE" NL );
      delay_ms( 10 );
    }

    ++n;
  }


  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

