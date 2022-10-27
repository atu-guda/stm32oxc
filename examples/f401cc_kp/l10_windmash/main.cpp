#include <oxc_auto.h>
#include <oxc_usartio.h> // TODO: auto

#include "uart_wm.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

PinsOut ledsx( GpioB, 12, 4 );

const char* common_help_string = "Widing machine control app" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

void idle_main_task()
{
  // leds.toggle( 1 );
}

// UART_CONSOLE_DEFINES
UART_HandleTypeDef uah_motordrv;
UsartIO motordrv( &uah_motordrv, USART1 );

STD_USART1_IRQ( motordrv );


int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR('t') = 1000;
  UVAR('n') =    2;

  ledsx.initHW();
  ledsx.reset( 0xFF );

  if( ! init_uart( &uah_motordrv ) ) {
    die4led( 1 );
  }

  motordrv.setHandleCbreak( false );
  devio_fds[5] = &motordrv;
  devio_fds[6] = &motordrv;
  motordrv.itEnable( UART_IT_RXNE );

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');
  std_out <<  "# Test0: n= " << n << " t= " << t_step << NL;

  char in_buf[80];

  // motordrv.enable();
  motordrv.reset();

  // log_add( "Test0 " );
  // uint8_t l_v = 0;
  uint32_t tm0 = HAL_GetTick();
  uint32_t tc0 = tm0, tc00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    motordrv.reset();
    uint32_t  tcb = HAL_GetTick();
    ledsx.set( 1 );
    auto w_n = motordrv.write( "ABCDE!", 6 );
    auto wr_ok = motordrv.wait_eot( 100 );
    ledsx.reset( 1 );
    // auto wr_ok = 1;

    delay_ms( 1 );
    memset( in_buf, '\x00', sizeof(in_buf) );
    auto r_n = motordrv.read( in_buf, 16, 10 );

    uint32_t  tcc = HAL_GetTick();
    std_out <<  "i= " << i << "  tick= " << ( tcc - tc00 ) << " dt = " << ( tcc - tcb )
            << " wr_ok=" << wr_ok << " r_n= " << r_n << " w_n= " << w_n 
            << ' ' << HexInt( motordrv.getSR() ) << NL;
    dump8( in_buf, 16 );

    // ++l_v;
    // l_v &= 0x0F;
    // ledsx.write( l_v );
    leds.toggle( 1 );

    delay_ms_until_brk( &tc0, t_step );
  }

  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

