#include <cstring>


#include <oxc_auto.h>


using namespace std;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

UART_HandleTypeDef uah;
int out_uart( const char *d, unsigned n );
void UART_handleIRQ();

// -------------------------------------------------------------------


class RingBuf {
  public:
   RingBuf( char *a_b, unsigned a_cap ); // used external buf
   // may be with dynamic buffer ?
   RingBuf( const RingBuf &r ) = delete;
   RingBuf& operator=( const RingBuf &rhs ) = delete;
   unsigned size() const { return sz; } // w/o block!
   unsigned capacity() const { return sz; }
   void put( char c ); // blocks, wait
   bool tryPut( char c ); // noblocks, fail if busy
   bool waitPut( char c, uint32_t ms = 100 ); // wait + try
  protected:
   char *b;
   unsigned sz = 0;
   const unsigned cap;
   mu_t mu = 0;
};

RingBuf::RingBuf( char *a_b, unsigned a_cap )
  : b( a_b ), cap( a_cap )
{
}

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

  char cn = '0';
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

