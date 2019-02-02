#include <cstring>

#include <oxc_auto.h>
#include <oxc_statdata.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test math statistics (StatData)" NL;

using sreal = StatData::sreal;

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
  leds.toggle( 1 );
}



int main(void)
{
  BOARD_PROLOG;

  UVAR('a') =   2;
  UVAR('b') =  10;
  UVAR('t') = 100;
  UVAR('n') = 1000000;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

int cmd_test0( int argc, const char * const * argv )
{
  STDOUT_os;
  unsigned n_ch = 2;
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 100000000 ); // number of series

  StatData sdat( n_ch );

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    // uint32_t tcc = HAL_GetTick();
    // if( i == 0 ) {
    //   tm0 = tcc; tm00 = tm0;
    // }

    sreal v[n_ch];
    v[0] = 1.0f * UVAR('a') +         ( ( i & 1 ) ? UVAR('b') : (-UVAR('b') ) );
    v[1] = 5.1f * UVAR('a') - 12.3f * ( ( i & 1 ) ? UVAR('b') : (-UVAR('b') ) );

    sdat.add( v );

    // delay_ms_until_brk( &tm0, t_step );
  }


  sdat.calc();
  os << sdat << NL;

  delay_ms( 10 );

  return 0;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

