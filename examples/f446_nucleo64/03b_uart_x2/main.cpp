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

// TODO: classes for mu_t handling: movew to oxc_main or oxc_mu

class MuLock {
  public:
   MuLock( mu_t &a_mu ) : mu( a_mu ) { mu_lock( &mu ); };
   ~MuLock()  { mu_unlock( &mu ); };
  protected:
   mu_t &mu;
};

class MuTryLock {
  public:
   MuTryLock( mu_t &a_mu ) : mu( a_mu ), acq( mu_trylock( &mu ) ) { };
   ~MuTryLock()  { if( acq ) { mu_unlock( &mu ); } };
   bool wasAcq() const { return acq; }
  protected:
   mu_t &mu;
   const bool acq;
};

class MuWaitLock {
  public:
   MuWaitLock( mu_t &a_mu, uint32_t ms = 100 ) : mu( a_mu ), acq( mu_waitlock( &mu, ms ) ) { };
   ~MuWaitLock()  { if( acq ) { mu_unlock( &mu ); } };
   bool wasAcq() const { return acq; }
  protected:
   mu_t &mu;
   const bool acq;
};

class RingBuf {
  public:
   RingBuf( char *a_b, unsigned a_cap ); // used external buf
   // may be with dynamic buffer ?
   RingBuf( const RingBuf &r ) = delete;
   RingBuf& operator=( const RingBuf &rhs ) = delete;
   unsigned size() const { return sz; } // w/o block!
   unsigned capacity() const { return cap; }
   bool put( char c ); // blocks, wait
   bool tryPut( char c ); // noblocks, fail if busy
   bool waitPut( char c, uint32_t ms = 100 ); // wait + try, delay_ms(1)
   unsigned puts( const char *s ); // blocks, in one lock, ret: number of char
   unsigned puts( const char *s, unsigned l ); // blocks, given length
   unsigned tryPuts( const char *s ); // non-blocks
   unsigned tryPuts( const char *s, unsigned l ); // non-blocks, given length
   pair<char,bool> get(); // blocks
   pair<char,bool> tryGet(); // noblocks
   pair<char,bool> waitGet( uint32_t ms = 100 ); // wait + try
   void reset() { MuLock lock( mu ); reset_nolock(); };
   void reset_nolock() { s = e = sz = 0; }
  protected:
   bool put_nolock( char c );
   pair<char,bool> get_nolock();
  protected:
   char *b;
   const unsigned cap; //* capacity
   unsigned sz = 0;
   unsigned s = 0;  //* start index
   unsigned e = 0;  //* end index
   // unsigned nf = ; //* number of inserted points [0;nb-1]
   // unsigned ni; //* number of insertion after sum recalc
   mu_t mu = 0;
};

RingBuf::RingBuf( char *a_b, unsigned a_cap )
  : b( a_b ), cap( a_cap )
{
}

bool RingBuf::put_nolock( char c )
{
  unsigned sn = s + 1;
  if( sn >= cap ) {
    sn = 0;
  }
  if( sn >= e ) {
    return false;
  }
  s = sn; b[s] = c; ++sz;
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

unsigned RingBuf::puts( const char *s )
{
  MuLock lock( mu );
  unsigned r;
  for( ; *s && ( r = put_nolock( *s ) ) ; ++s ) {
  }
  return r;
}

unsigned RingBuf::puts( const char *s, unsigned l )
{
  MuLock lock( mu );
  unsigned r;
  for( r=0; r<l && put_nolock( *s ) ; ++s, ++r ) {
  }
  return r;
}

unsigned RingBuf::tryPuts( const char *s )
{
  MuTryLock lock( mu );
  if( ! lock.wasAcq() ) {
    return 0;
  }
  unsigned r;
  for( r=0; *s && put_nolock( *s ) ; ++s, ++r ) {
  }
  return r;
}

unsigned RingBuf::tryPuts( const char *s, unsigned l )
{
  MuTryLock lock( mu );
  if( ! lock.wasAcq() ) {
    return 0;
  }
  unsigned r;
  for( r=0; r<l && put_nolock( *s ) ; ++s, ++r ) {
  }
  return r;
}


pair<char,bool> RingBuf::get_nolock()
{
  if( s == e ) {
    return make_pair( '\0', false );
  }
  char c = b[e++];
  if( e >= cap ) {
    e = 0;
  }
  --sz;
  return make_pair( c, true );
}


pair<char,bool> RingBuf::get()
{
  MuLock lock( mu );
  return get();
}

pair<char,bool> RingBuf::tryGet()
{
  MuTryLock lock( mu );
  if( lock.wasAcq() ) {
    return get_nolock();
  }
  return make_pair( '\0', false );
}

pair<char,bool> RingBuf::waitGet( uint32_t ms )
{
  MuWaitLock lock( mu, ms );
  if( lock.wasAcq() ) {
    return get_nolock();
  }
  return make_pair( '\0', false );
}

// -------------------------------------------------------------------


class DevIO {
  public:
   using OnRecvFun = void (*)( const char *s, int l ); // to async reaction
   using SigFun = void (*)( int v );
   enum {
     TX_BUF_SIZE = 256, //* low-level transmit buffer size
     RX_BUF_SIZE = 256  //* low-level receive buffer size, buffer itself - only if required
   };

   DevIO()
    :obuf( tx_buf_xx, sizeof( tx_buf_xx ) ),
     ibuf( rx_buf_xx, sizeof( rx_buf_xx ) )
    {};
   virtual ~DevIO();
   virtual void reset();
   virtual int getErr() const { return err; }
   void setWaitTx( int tx ) { wait_tx = tx; }
   void setWaitRx( int rx ) { wait_rx = rx; }

   virtual int sendBlock( const char *s, unsigned l );
   virtual int sendBlockSync( const char *s, unsigned l ) = 0;
   virtual int sendStr( const char *s );
   virtual int sendStrSync( const char *s );
   int sendByte( char b ) { return sendBlock( &b, 1 ); };
   int sendByteSync( char b ) { return sendBlockSync( &b, 1 ); };
   int sendInt16( int16_t v ) { return sendBlock( (const char*)(&v), sizeof(int16_t) ); };
   int sendInt16Sync( int16_t v ) { return sendBlockSync( (const char*)(&v), sizeof(int16_t) ); };
   int sendInt32( int32_t v ) { return sendBlock( (const char*)(&v), sizeof(int32_t) ); };
   int sendInt32Sync( int32_t v ) { return sendBlockSync( (const char*)(&v), sizeof(int32_t) ); };

   virtual int recvByte( char *s, int w_tick = 0 );
   virtual int recvBytePoll( char *s, int w_tick = 0 ) = 0;
   virtual int recvBlock( char *s, int l, int w_tick = 0 ); // w_tick - for every
   virtual int recvBlockPoll( char *s, int l, int w_tick = 0 );
   void setOnRecv( OnRecvFun a_onRecv ) { onRecv = a_onRecv; };
   void setOnSigInt( SigFun a_onSigInt ) { onSigInt = a_onSigInt; };

   virtual void task_send();
   virtual void task_recv();
   void charsFromIrq( const char *s, int l );

   virtual int  setAddrLen( int addrLen ) = 0;
   virtual int  getAddrLen() const = 0;
   virtual int  setAddr( uint32_t addr ) = 0;
   // void initIRQ( uint8_t ch, uint8_t prio ); // TODO:? store ch
  protected:
   // TODO: open mode + flags
   char tx_buf_xx[TX_BUF_SIZE];
   char rx_buf_xx[RX_BUF_SIZE];
   RingBuf obuf;
   RingBuf ibuf;
   OnRecvFun onRecv = nullptr;
   SigFun onSigInt = nullptr;
   int err = 0;
   int wait_tx = 1500;
   int wait_rx = 1500;
   bool on_transmit = false;
   bool blocking_send = true;
   bool blocking_recv = false;
};

DevIO::~DevIO()
{
  reset();
}

void DevIO::reset()
{
  ibuf.reset();
  obuf.reset();
}


int DevIO::sendBlock( const char *s, unsigned l )
{
  if( !s  ||  l < 1 ) {
    return 0;
  }

  bool ok;
  if( blocking_send ) {
    ok = obuf.puts( s, l );
  } else {
    ok = obuf.tryPuts( s, l );
  }


  // if( ns ) {
  //   taskYieldFun();
  // }

  return ok;
}

int DevIO::recvByte( char *b, int w_tick )
{
  if( !b ) { return 0; }

  char c;
  BaseType_t r = ibuf.recv( &c, w_tick );
  if( r == pdTRUE ) {
    *b = c;
    return 1;
  }

  return 0;
}

void DevIO::task_send()
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
  sendBlockSync( tx_buf, ns ); // TODO: if( send_now )?
};

void DevIO::task_recv()
{
  // leds.set( BIT2 );
  char cr;
  BaseType_t ts = ibuf.recv( &cr, wait_rx );
  if( ts == pdTRUE ) {
    if( onRecv != nullptr ) {
      onRecv( &cr, 1 );
    } else {
      // else simply eat char - if not required - dont use this task
    }
  }
  // leds.reset( BIT2 );
}

void DevIO::charsFromIrq( const char *s, int l ) // called from IRQ!
{
  BaseType_t wake = pdFALSE;
  for( int i=0; i<l; ++i ) {
    if( s[i] == 3  && onSigInt ) { // handle Ctrl-C = 3
      onSigInt( s[i] );
    }
    ibuf.sendFromISR( s+i, &wake  );
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



// -------------------------------------------------------------------

// -------------------------------------------------------------------

int out_uart( const char *d, unsigned n )
{
  return HAL_UART_Transmit( &uah, (uint8_t*)d, n, 10 ) == HAL_OK;
}

void BOARD_UART_DEFAULT_IRQHANDLER(void) {
  leds.toggle( BIT3 ); // DEBUG
  UART_handleIRQ();
  HAL_UART_IRQHandler( &uah );
}

volatile char in_char = ' ';

void UART_handleIRQ()
{
  uint16_t status = BOARD_UART_DEFAULT->USART_SR_REG;
  bool on_transmit = false;

  // leds.toggle( BIT3 ); // DEBUG

  if( status & UART_FLAG_RXNE ) { // char recived
    leds.set( BIT0 );
    // ++n_work;
    // char cr = recvRaw();
    in_char = BOARD_UART_DEFAULT->USART_RX_REG;
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

// extern "C" {
// void* malloc( unsigned sz );
// }


int main(void)
{
  STD_PROLOG_UART_NOCON;

  char ring_x_buf[128];
  RingBuf tring( ring_x_buf, sizeof( ring_x_buf ) );
  auto z = tring.tryGet();

  char cn = '0';
  if( z.second ) {
    cn = z.first;
  }

  leds.write( 0 );
  MSTRF( os, 128, out_uart );

  uint32_t c_msp = __get_MSP();
  os << " MSP-__heap_top = " << ((unsigned)c_msp - (unsigned)(__heap_top) ) << "\r\n";
  os.flush();
  os << "CR1: " << HexInt16( BOARD_UART_DEFAULT->CR1 )  << "  CR2: " << HexInt16( BOARD_UART_DEFAULT->CR2 )  << "\r\n";
  os.flush();

  // char *tmp = (char*)malloc(128);
  // char *tmp = new char[128];

  int n = 0;

  // BOARD_UART_DEFAULT->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE  | USART_CR1_TXEIE;

  while( 1 ) {
    bool was_action = false;
    char c = '?', c_err = '-';
    uint16_t u_sr = BOARD_UART_DEFAULT->USART_SR_REG;
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
         << in_char << "> " << HexInt16( u_sr ) << " -\r\n";
      os.flush();
    } else if( n % ( 50*delay_calibrate_value ) == 0 ) {
      leds.toggle( BIT1 );
      os << '-' << cn << ' ' << HexInt16( u_sr ) << " -\r\n";
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

