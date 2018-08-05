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

// -------------------------------------------------------------------



int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};



int main(void)
{
  STD_PROLOG_UART; // <oxc_base.h>

  dev_console.sendStrSync( NL "0123456789ABCDEF" NL  );

  UVAR('t') = 100;
  UVAR('n') =  20;

  leds.write( 0 );

  pr( PROJ_NAME NL );

  srl.re_ps();
  OxcTicker led_tick( &task_leds_step, TASK_LEDS_QUANT );

  while( 1 ) {

    auto v = dev_console.tryGet();

    if( v.good() ) {
      srl.addChar( v.c );
    } else {
      delay_ms( 10 );
    }

    if( led_tick.isTick() ) {
      leds.toggle( LED_BSP_IDLE );
    }

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

