#include <cstring>
#include <iterator>
#include <algorithm>

#include <cmath>

#include "mjs.h"

#include <oxc_auto.h>
#include <oxc_statdata.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

int js_cmdline_handler( char *s );

const char* common_help_string = "Appication to test mjs" NL;

// --- local commands;
int cmd_test0( int argc, const char * const * argv );
CmdInfo CMDINFO_TEST0 { "test0", 'T', cmd_test0, "\"cmd ...\" - test exec"  };
int cmd_resetjs( int argc, const char * const * argv );
CmdInfo CMDINFO_RESETJS { "resetjs", 'Z', cmd_resetjs, " - reset js engine"  };

const CmdInfo* global_cmds[] = {
  DEBUG_CMDS,

  &CMDINFO_TEST0,
  &CMDINFO_RESETJS,
  nullptr
};

mjs *js = nullptr;
mjs_val_t arr1 = MJS_UNDEFINED;
mjs_val_t js_Math = MJS_UNDEFINED;
void resetjs();
void mjs_f1( mjs *mjs );

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

constexpr const NamedObj *const fl01_objs[] = {
  & fl0_W_max,
  & fl0_V_max,
  nullptr
};

const NamedObjs fl01( fl01_objs );


constexpr NamedSubObj fl0_sub    {     "sub",       &fl01 };
constexpr NamedFloat fl0_X_c     {     "X_c",       &X_c,  1, NamedFloat::Flags::ro  };
constexpr NamedFloat fl0_pwm_min { "pwm_min",   get_pmin, set_pmin  };
constexpr NamedFloat fl0_va      {      "va",         va,  size(va) };

constexpr NamedInt   fl0_iv      {   "iv",          &iv  };
constexpr NamedInt   fl0_iva     {   "iva",         iva, size(iva) };
constexpr NamedInt   fl0_ivf     {   "ivf",     get_ivf, set_ivf };

constexpr const NamedObj *const fl0_objs[] = {
  & fl0_sub,
  & fl0_X_c,
  & fl0_pwm_min,
  & fl0_va,
  & fl0_iv,
  & fl0_iva,
  & fl0_ivf,
  nullptr
};

const NamedObjs fl0( fl0_objs );

bool print_var_fl( const char *nm, int fmt )
{
  return fl0.print( nm, fmt );
}

bool set_var_fl( const char *nm, const char *s )
{
  auto ok =  fl0.set( nm, s );
  print_var_fl( nm, 0 );
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

  cmdline_handlers[0] = js_cmdline_handler;
  cmdline_handlers[1] = nullptr;

  print_var_hook    = print_var_fl;
  set_var_hook      = set_var_fl;

  BOARD_POST_INIT_BLINK;
  resetjs();

  std_out << NL "##################### " PROJ_NAME NL;


  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  leds.reset( 0xFF );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

int cmd_test0( int argc, const char * const * argv )
{
  const char *cmd = ( argc > 1 ) ? argv[1] : "let f =  7 + 8; f;";

  std_out << "# Test: cmd= \"" << cmd << '"' << NL;
  delay_ms( 10 );

  mjs_val_t ret /* = MJS_UNDEFINED */;
  mjs_err_t rc = mjs_exec( js, cmd, &ret );

  std_out << "# rc= " << rc << " ret= " << HexInt64( ret, true ) << NL;
  if( rc != 0 ) {
    std_out << "# err: " << mjs_strerror( js, rc ) << NL;
  }


  return 0;
}

int js_cmdline_handler( char *s )
{
  static int nnn = 0;

  if( s[0] == '!' ) {
    const char *cmd = s + 1;
    std_out << NL "# JS: cmd= \"" << cmd << '"' << NL;
    delay_ms( 10 );

    mjs_val_t ret = MJS_UNDEFINED;
    ++nnn;

    mjs_array_set( js, arr1, 1, mjs_mk_number( js, nnn ) );
    mjs_err_t rc = mjs_exec( js, cmd, &ret );

    std_out << "# rc= " << rc << " ret= " << HexInt64( ret, true ) << NL;
    if( rc != 0 ) {
      std_out << "# err: " << mjs_strerror( js, rc ) << NL;
    } else {
      if( mjs_is_number( ret ) ) {
        std_out << "# ret: " << mjs_get_int( js, ret ) << NL;
      }
    }
    return rc;
  }

  return -1;
}

void mjs_f1( mjs *mjs )
{
  size_t num_args = mjs_nargs( mjs );
  for( size_t i = 0; i < num_args; i++ ) {
    auto arg = mjs_arg( mjs, i );
    mjs_fprintf( arg, mjs, stdout );
    std_out << ' ';
  }
  std_out << NL;

  if( num_args > 0 ) {
    auto arg0 = mjs_arg( mjs, 0 );
    if( mjs_is_object( arg0 ) ) {
      mjs_val_t key, iter = MJS_UNDEFINED;
      while( ( key = mjs_next( js, arg0, &iter ) ) != MJS_UNDEFINED) {
       mjs_fprintf( key, js, stdout );
      }
    }
  }
  mjs_return( mjs, MJS_UNDEFINED );
}

void js_Math_sin( mjs *mjs )
{
  size_t num_args = mjs_nargs( mjs );
  if( num_args < 1 ) {
    mjs_return( mjs, MJS_UNDEFINED );
    return;
  }

  auto arg = mjs_arg( mjs, 0 );
  // if( ! mjs_is_number( arg ) ) {
  //   mjs_return( mjs, MJS_UNDEFINED );
  //   return;
  // }

  double a = mjs_get_double( js, arg );
  double r = sin( a );

  mjs_return( mjs, mjs_mk_number( js, r ) );
}

void resetjs()
{
  if( js ) {
    mjs_destroy( js );
  }
  js = mjs_create();
  mjs_val_t global = mjs_get_global( js );
  mjs_set( js, global, "f1", ~0, mjs_mk_foreign_func( js, (mjs_func_ptr_t) mjs_f1 ) );
  // mjs_val_t obj1 = mjs_( js );

  arr1 = mjs_mk_array( js );
  mjs_array_push( js, arr1, mjs_mk_number( js, 10 ) );
  mjs_array_push( js, arr1, mjs_mk_number( js, 11 ) );
  mjs_array_push( js, arr1, mjs_mk_number( js, 12 ) );
  mjs_array_push( js, arr1, mjs_mk_number( js, 13 ) );
  mjs_set( js, global, "arr1", 4, arr1 );

  js_Math = mjs_mk_object( js );
  mjs_set( js, global, "Math", ~0, js_Math );
  mjs_set( js, js_Math, "sin", ~0, mjs_mk_foreign_func( js, (mjs_func_ptr_t) js_Math_sin ) );
}

int cmd_resetjs( int argc, const char * const * argv )
{
  resetjs();
  return 0;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

