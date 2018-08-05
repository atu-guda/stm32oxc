#include <cstring>


#include <oxc_auto.h>
#include <oxc_usartio.h> // FOR vim open
#include <oxc_console.h>
#include <oxc_smallrl.h>
#include <oxc_debug1.h>

using namespace std;
using namespace SMLRL;


USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;  // (from bsp/BOARDNAME/board_cfg.h):
// =  UART_CONSOLE_DEFINES( USART2 )
// BOARD_UART_DEFAULT = USART2
// BOARD_UART_DEFAULT_NAME       "USART2"
// BOARD_UART_DEFAULT_IRQ        USART2_IRQn
// BOARD_UART_DEFAULT_GPIO_AF    GPIO_AF7_USART2
// BOARD_UART_DEFAULT_ENABLE     __USART2_CLK_ENABLE(); __GPIOA_CLK_ENABLE();
// BOARD_UART_DEFAULT_DISABLE    __USART2_CLK_DISABLE();
// BOARD_UART_DEFAULT_IRQHANDLER USART2_IRQHandler

// oxc_usartbase.h
//  void USART2_IRQHandler(void); extern "C"
//  void task_usart2_send( void *prm UNUSED_ARG );
//  void task_usart2_recv( void *prm UNUSED_ARG );
//  STD_USART2_IRQ( obj ) void USART2_IRQHandler(void) { obj.handleIRQ(); }


// oxc_usartio.h:
//  UART_CONSOLE_DEFINES( dev ) =
//    UART_HandleTypeDef uah_console;
//    UsartIO dev_console( &uah_console, dev );
//    STD_ ## dev ## _SEND_TASK( dev_console ); // --
//    STD_ ## dev ## _IRQ( dev_console );
//    SmallRL srl( smallrl_exec );
// #define STD_USART2_SEND_TASK( obj ) STD_COMMON_SEND_TASK( task_usart2_send, obj ) // --
// #define STD_USART2_RECV_TASK( obj ) STD_COMMON_RECV_TASK( task_usart2_recv, obj ) // --

// -------------------------------------------------------------------



int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};



int post_exec( int rc )
{
  dev_console.reset_in();
  return rc;
}



int main(void)
{
  STD_PROLOG_UART; // <oxc_base.h>

  dev_console.sendStrSync( NL "0123456789ABCDEF" NL  );

  UVAR('t') = 100;
  UVAR('n') =  20;

  leds.write( 0 );

  int n = 0;

  pr( PROJ_NAME NL );

  srl.setPostExecFun( post_exec );

  srl.re_ps();

  while( 1 ) {

    leds.toggle( BIT3 );
    auto v = dev_console.tryGet();

    if( v.good() ) {
      srl.addChar( v.c );
    } else {
      delay_ms( 10 );
    }

    ++n;
    // dev_console.sendBlock( "ABCD" NL, 6 );
    // delay_ms( 1000 );
  }


  return 0;
}

// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  STD_os;
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  os << NL "Test0: n= " <<  n <<  " t= " << t_step << NL;
  os.flush();

  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t tmc = HAL_GetTick();
    os << " Fake Action i= "  << i <<  " tick: " << ( tmc - tm00 ) << NL;
    os.flush();
    if( UVAR('w') ) {
       dev_console.wait_eot();
    }
    // delay_ms( 3 );
    delay_ms_until_brk( &tm0, t_step );
    // delay_ms_brk( t_step );
  }

  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

