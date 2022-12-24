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


int main(void)
{
  STD_PROLOG_UART;

  UVAR('t') = 100;
  UVAR('n') = 10;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );
  oxc_add_aux_tick_fun( aux_tick_fun2 );

  std_main_loop_nortos( &srl, nullptr );

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
    iprintf( " Fake Action i= %d  tick: %lu " NL, i, tmc - tm00 );

    delay_ms_until_brk( &tm0, t_step );
  }

  return 0;
}




// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

