#include <cstring>
#include <iterator>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_statdata.h>
#include <oxc_namedints.h>
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
int cmd_testsplit( int argc, const char * const * argv );
CmdInfo CMDINFO_TESTSPLIT { "testspit", 'X', cmd_testsplit, " expr - test var split"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_TESTOUT,
  &CMDINFO_TESTSPLIT,
  nullptr
};

void idle_main_task()
{
  leds.toggle( 1 );
}

// ---------------------- ints + floats iface ---------------------------


float W_max = 100.0f;
float V_max =   8.0f;
float X_c   =   1.34f;
float pmin  =   5.0f;
float va[4] = { 0.1f, -0.2f, 0.3f, -0.5f };

float get_pmin( int /*idx*/ )
{
  return pmin;
}

bool set_pmin( float v, int /* idx */ )
{
  pmin = clamp( v, 2.0f, 90.0f );
  return true;
}

int iv = 42;
int iva[] = { -7, 8, 9, -10, 15 };
int ivf = 17;

int get_ivf( int /*idx*/ )
{
  return ivf;
}

bool set_ivf( int v, int /* idx */ )
{
  ivf = clamp( v, -20, 20 ) & (~1u);
  return true;
}


constexpr NamedFloat fl0_W_max   {   "W_max",     &W_max  };
constexpr NamedFloat fl0_V_max   {   "V_max",     &V_max  };
constexpr NamedFloat fl0_X_c     {     "X_c",       &X_c,  1, NamedFloat::Flags::ro  };
constexpr NamedFloat fl0_pwm_min { "pwm_min",   get_pmin, set_pmin  };
constexpr NamedFloat fl0_va      {      "va",         va,  size(va) };

constexpr NamedInt   fl0_iv      {   "iv",          &iv  };
constexpr NamedInt   fl0_iva     {   "iva",         iva, size(iva) };
constexpr NamedInt   fl0_ivf     {   "ivf",     get_ivf, set_ivf };

constexpr const NamedObj *const fl0_objs[] = {
  & fl0_W_max,
  & fl0_V_max,
  & fl0_X_c,
  & fl0_pwm_min,
  & fl0_va,
  & fl0_iv,
  & fl0_iva,
  & fl0_ivf,
  nullptr
};

NamedObjs fl0( fl0_objs );

bool print_var_fl( const char *nm )
{
  return fl0.print( nm );
}

bool set_var_fl( const char *nm, const char *s )
{
  auto ok =  fl0.set( nm, s );
  print_var_fl( nm );
  return ok;
}

const char* get_var_name_fl( unsigned i )
{
  return fl0.getName( i );
}
// ---------------------------------------------------------



int main(void)
{
  BOARD_PROLOG;

  UVAR('a') =   2;
  UVAR('b') =  10;
  UVAR('t') = 100;
  UVAR('n') = 1000000;

  print_var_hook    = print_var_fl;
  set_var_hook      = set_var_fl;
  get_var_name_hook = get_var_name_fl;

  BOARD_POST_INIT_BLINK;

  STDOUT_os;
  os << NL "##################### " PROJ_NAME NL;
  // os << "# fl0.size = " << fl0.size() << " name= \"" << fl0.getName( 1 ) << "\"" NL;
  // os << "# &fl0_W_max  = " << HexInt( (void*)&fl0_W_max ) << " fl0_objs= "  << HexInt( (void*)fl0_objs )
  //    << " fl0.begin()= " << HexInt( (void*)( fl0.begin() ) ) << NL;
  //
  // auto f = fl0.begin();
  // os << "# f.getName()= \"" << f->getName() << "\"" NL;




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

int cmd_testsplit( int argc, const char * const * argv )
{
  STDOUT_os;
  if( argc < 2 ) {
    os << "# Error: expresssion required! " NL;
    return 1;
  }

  int idx;
  char nm[maxSimpleNameLength];
  const char *eptr;
  bool ok = splitNameWithIdx( argv[1], nm, idx, &eptr );
  os << "ok= " << ok << " nm=\"" << nm << "\" idx= " << idx << " *eptr='" << (*eptr) << '\'' <<NL;
  fl0.print( argv[1] );
  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

