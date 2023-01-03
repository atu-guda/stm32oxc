#include <stdio.h>

#include <oxc_auto.h>

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


void xxx_main_loop_nortos( SmallRL *sm, AuxTickFun f_idle )
{
  if( !sm ) {
    die4led( 0 );
  }

  UVAR('i') = 0;

  // eat pre-input
  reset_in( 0 );
  for( unsigned i=0; i<256; ++i ) {
    ++UVAR('i');
    auto v = tryGet( 0 );
    if( v.empty() ) {
      break;
    }
  }

  while( 1 ) {
    auto v = tryGet( 0 );

    if( v.good() ) {
      sm->addChar( v.c );
    } else {
      if( f_idle ) {
        f_idle();
      }
      delay_ms( UVAR('q') );
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

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );
  oxc_add_aux_tick_fun( aux_tick_fun2 );

  xxx_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n = arg2long_d( 1, argc, argv, UVAR('n'), 0 );
  uint32_t t_step = UVAR('t');

  std_out
    << "# ctrl= "      << HexInt(__get_CONTROL())
    << " apsr= "       << HexInt(__get_APSR())
    << " ipsr= "       << HexInt(__get_IPSR())
    << " xpsr= "       << HexInt(__get_xPSR())
    << " ipsr= "       << HexInt(ixpsr)
    << " primask= "    << HexInt(__get_PRIMASK())
    << NL;


  uint32_t tm0 = HAL_GetTick(), tm00 = tm0;

  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {

    // action
    uint32_t tmc = HAL_GetTick();
    std_out << i << ' ' << (tmc - tm00) << ' ' << tm0 << NL;

    switch( UVAR('l') ) {
      case 0:  delay_ms_brk( t_step );             break;
      case 1:  delay_ms_until_brk( &tm0, t_step ); break;
      case 2:  delay_ms(  t_step );                break;
      case 3:  HAL_Delay( t_step );                break;
      case 4:  delay_ms_until_brk_ex( nullptr, t_step, false ); break;
      default: break; // no delay ;-)
    }
  }

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

