#include <cstring>
#include <cstdlib>

#include <oxc_auto.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;


// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };
int cmd_test1( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST1 { "test1", 'X', cmd_test1, " - test something 1"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TEST1,
  nullptr
};


int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR('t') = 100;
  UVAR('n') = 20;

  BOARD_POST_INIT_BLINK;


  leds.reset( 0xFF );

  srl.re_ps();
  srl.reset();
  OxcTicker led_tick( &task_leds_step, TASK_LEDS_QUANT );

  pr( NL "------------------ " PROJ_NAME NL );
  delay_ms( 10 );
  dev_console.reset_in();

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

// TEST1
int cmd_test1( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  char b[128];
  if( n >= (int)sizeof(b) ) {
    n = sizeof(b)-1;
  }
  for( int i=0; i<n; ++i ) {
    b[i] = 'X';
  }
  dev_console.write_s( b, n );
  delay_ms( 10 );
  dev_console.write_s( NL, 2 );
  delay_ms( 10 );
  dev_console.write( b, n );
  delay_ms( 10 );
  dev_console.write_s( NL, 2 );
  delay_ms( 10 );

  return 0;
}






// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

