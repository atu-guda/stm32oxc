#include <cstring>


#include <oxc_auto.h>
#include <oxc_console.h>
#include <oxc_smallrl.h>
#include <oxc_debug1.h>
#include <oxc_main.h>


using namespace std;
using namespace SMLRL;


USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

UART_HandleTypeDef uah_console;
int out_uart( const char *d, unsigned n );
void UART_handleIRQ();

// -------------------------------------------------------------------



DCL_CMD_REG( test0, 'T', " - test UART"  );


const unsigned buf_sz = 128;

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
    if( /* handle_cbreak && */ in_char == 3 ) {
      break_flag = 1;
    }
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

void out( const char *s, unsigned l )
{
  if( !s || !*s ) {
    return;
  }

  if( tx_ring.puts_ato( s, l ) > 0 ) {
    start_transmit();
    return;
  }

  leds.set( BIT2 );
  tx_ring.tryPut( *s++ );
  start_transmit();
  tx_ring.puts( s, l );
  leds.reset( BIT2 );
}


void out( const char *s )
{
  out( s, strlen(s) );
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
    delay_mcs( 100 );
  }
}

int prl( const char *s, unsigned l,  int /* fd */ )
{
  out( s, l );
  return l;
}

int prl1( const char *s, unsigned l )
{
  out( s, l );
  return l;
}

int pr( const char *s,  int /*fd = 1*/   )
{
  return prl( s, strlen(s) );
}

int post_exec( int rc )
{
  rx_ring.reset();
  return rc;
}


int main(void)
{
  STD_PROLOG_UART_NOCON;

  UVAR_t = 100;
  UVAR_n =  20;

  leds.write( 0_mask );

  int n = 0;


  oxc_add_aux_tick_fun( start_transmit );

  tx_ring.set_n_wait( 1000 );

  pr( PROJ_NAME NL );


  BOARD_UART_DEFAULT->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;

  SmallRL cworker( exec_direct );
  cworker.setPostExecFun( post_exec );

  cworker.re_ps();

  while( 1 ) {

    // leds.toggle( BIT3 );

    auto rec = rx_ring.get();

    if( rec.good() ) {
      cworker.addChar( rec.c );
    } else {
      delay_ms( 10 );
    }

    ++n;
  }


  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR_n, 0 );
  uint32_t t_step = UVAR_t;
  pr( NL "Test0: n= " ); pr_d( n ); pr( " t= " ); pr_d( t_step );
  pr( NL );

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t tmc = HAL_GetTick();
    pr( " Fake Action i= " ); pr_d( i );
    pr( "  ms_tick: "); pr_d( tmc - tm00 );
    pr( NL );
    if( UVAR_w ) {
      wait_eot();
    }
    // delay_ms( 3 );
    delay_ms_until_brk( &tm0, t_step );
    // delay_ms_brk( t_step );
  }

  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

