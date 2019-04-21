#include <cstring>
#include <iterator>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_statdata.h>
#include <oxc_namedfloats.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test misc math" NL;

using sreal = StatData::sreal;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, " - test statistics"  };
int cmd_testout( int argc, const char * const * argv );
CmdInfo CMDINFO_TESTOUT { "testout", 'O', cmd_testout, " - test output"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TESTOUT,
  nullptr
};

void idle_main_task()
{
  leds.toggle( 1 );
}

// ---------------------- floats iface ---------------------------

float W_max = 100.0f;
float V_max =   8.0f;
float X_c   =   1.34f;
float pmin  =   5.0f;
float va[4] = { 0.1f, -0.2f, 0.3f, -0.5f };

float get_pmin()
{
  return pmin;
}

bool set_pmin( float v )
{
  pmin = clamp( v, 2.0f, 90.0f );
  return true;
}


const NamedFloat flts[] = {
  {      "W_max",              &W_max,      nullptr,      nullptr, NamedFloat::Flags::flg_no, 1  },
  {      "V_max",              &V_max,      nullptr,      nullptr, NamedFloat::Flags::flg_no, 1  },
  {        "X_c",                &X_c,      nullptr,      nullptr, NamedFloat::Flags::flg_ro, 1  },
  {    "pwm_min",             nullptr,     get_pmin,     set_pmin, NamedFloat::Flags::flg_no, 1  },
  {         "va",                  va,      nullptr,      nullptr, NamedFloat::Flags::flg_no, size(va)  },
  {      nullptr,             nullptr,      nullptr,      nullptr, NamedFloat::Flags::flg_no, 0  }
};

NamedFloats fl( flts );

// ---------------------------------------------------------



int main(void)
{
  BOARD_PROLOG;

  UVAR('a') =   2;
  UVAR('b') =  10;
  UVAR('t') = 100;
  UVAR('n') = 1000000;

  NamedFloats::set_global_floats( &fl );
  print_var_hook    = NamedFloats::g_print;
  set_var_hook      = NamedFloats::g_fromText;
  get_var_name_hook = NamedFloats::g_getName;

  BOARD_POST_INIT_BLINK;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  leds.reset( 0xFF );

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

  return 0;
}

int cmd_testout( int argc, const char * const * argv )
{
  STDOUT_os;

  if( argc > 1 ) {
    float f = arg2float_d( 1, argc, argv, 1.234f, -FLT_MAX, FLT_MAX  );
    os <<        FltFmt( f )
       << ' ' << FltFmt( f, cvtff_exp )
       << ' ' << FltFmt( f, cvtff_fix )
       << NL;
    return 0;
  }

  for( float f = 7.23456789e-12f; f < 1e14f; f *= -10 ) {
    os <<        FltFmt( f )
       << ' ' << FltFmt( f, cvtff_exp )
       << ' ' << FltFmt( f, cvtff_fix )
       << NL;
  }
  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

