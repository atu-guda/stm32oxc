#include <stdio.h>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_ticker.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES_UART;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test something 0"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  nullptr
};

volatile uint32_t ixpsr {0};


void aux_tick_fun2(void)
{
  ixpsr = __get_xPSR();
}

int delay_led_step { 500 };

int on_delay_actions()
{
  static OxcTicker delay_tick( &delay_led_step, 1 );
  if( delay_tick.isTick() ) {
    leds.toggle( BIT2 );
  }
  return 0;
}


void xxx_main_loop_nortos( SmallRL *sm, AuxTickFun f_idle )
{
  if( !sm ) {
    die4led( 0 );
  }

  UVAR('i') = 0;

  // eat pre-input
  reset_in( 0 );
  for( unsigned i=0; i<256; ++i ) {
    auto v = tryGet( 0 );
    if( v.empty() ) {
      break;
    }
    ++UVAR('i');
  }

  srl.re_ps(); srl.reset();

  while( true ) {
    leds.set( 2 );
    auto v = tryGet_irqdis( 0 );
    leds.reset( 2 );

    if( v.good() ) {
      sm->addChar( v.c );
    } else {
      if( f_idle ) {
        f_idle();
      }

      if( UVAR('q') > 0 ) {
        delay_ms( UVAR('q') );
      } else {
        on_delay_actions();
      }

    }
  }
}

int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') = 100;
  UVAR('n') =  10;
  UVAR('q') =   0;
  UVAR('l') =   0; // delay type

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME " #####################"  NL );


  oxc_add_aux_tick_fun( led_task_nortos );
  oxc_add_aux_tick_fun( aux_tick_fun2 );

  xxx_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = arg2long_d( 2, argc, argv, UVAR('t'), 0, 100000 );
  int tp = arg2long_d( 3, argc, argv, UVAR('v'), 0, 10 );
  std_out <<  "# test_delays : n= " << n << " t= " << t_step << " tp= " << tp << NL;

  std_out
    << "# ctrl= "      << HexInt(__get_CONTROL())
    << " apsr= "       << HexInt(__get_APSR())
    << " ipsr= "       << HexInt(__get_IPSR())
    << " xpsr= "       << HexInt(__get_xPSR())
    << " ipsr= "       << HexInt(ixpsr)
    << " primask= "    << HexInt(__get_PRIMASK())
    << NL;

  test_delays_misc( n, t_step, tp );

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

