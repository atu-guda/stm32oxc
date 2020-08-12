#include <cstring>
#include <iterator>
#include <algorithm>

#include <cmath>

#include <oxc_auto.h>
#include <oxc_statdata.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>

#include <oxc_mjs.h>


using namespace std;
using namespace SMLRL;
using namespace OXC_MJS;

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

struct Ffi_ptrs {
  const char *const name;
  void *const ptr;
};

Mjs *js = nullptr;
mjs_val_t arr1 = MJS_UNDEFINED;
// mjs_val_t js_Math = MJS_UNDEFINED;
void resetjs();
void mjs_f1( Mjs *mjs );
extern const char js_Math_funcs[];
extern const Ffi_ptrs ffi_ptrs[];
void *mjs_dlsym_local( void *handle, const char *name );
using F_D_D = double(*)(double);
using F_D_DD = double(*)(double,double);
#define FUN_D_D(x)  (void*)(F_D_D)(x)
#define FUN_D_DD(x) (void*)(F_D_DD)(x)




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
  const char *cmd = ( argc > 1 ) ? argv[1] : "let f =  7 + 8; f; let a = Math.sin(0.1); print( a );";

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
      auto ar_ret = mjs_array_get( js, arr1, 2 );
      if( mjs_is_number( ar_ret ) ) {
        std_out << "# ar_ret: " << mjs_get_double( js, ar_ret ) << NL;
      }
      if( mjs_is_number( ret ) ) {
        std_out << "# ret: " << mjs_get_int( js, ret ) << NL;
      }
    }
    return rc;
  }

  return -1;
}

void mjs_f1( Mjs *mjs )
{
  char buf[128];
  size_t num_args = mjs_nargs( mjs );
  for( size_t i = 0; i < num_args; i++ ) {
    auto arg = mjs_arg( mjs, i );
    mjs_sprintf( arg, mjs, buf, sizeof(buf)-1 );
    std_out << "# i= " << i << " s= \"" << buf << "\"" NL;
  }

  if( num_args > 0 ) {
    auto arg0 = mjs_arg( mjs, 0 );
    if( Mjs::is_object( arg0 ) ) {
      mjs_val_t key, iter = MJS_UNDEFINED;
      while( ( key = mjs_next( js, arg0, &iter ) ) != MJS_UNDEFINED) {
        mjs_sprintf( key, js, buf, sizeof(buf)-1 );
        std_out << "# xx  s= \"" << buf << "\"" NL;
      }
    }
  }
  mjs_return( mjs, MJS_UNDEFINED );
}

void js_Math_sin( Mjs *mjs )
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
    delete js; js = nullptr;
  }
  js = new Mjs; // mjs_create();
  mjs_val_t global = js->get_global();
  js->set( global, "f1", ~0, mjs_mk_foreign_func( js, (mjs_func_ptr_t) mjs_f1 ) );
  mjs_set_ffi_resolver( js, mjs_dlsym_local );
  // mjs_val_t obj1 = mjs_( js );

  const unsigned arr1_sz = 4;
  arr1 = js->mk_array();
  for( decltype(+arr1_sz) i=0; i< arr1_sz; ++i ) {
    mjs_array_push( js, arr1, mjs_mk_number( js, i + 10 ) );
  }
  js->set( global, "arr1", arr1_sz, arr1 );

  mjs_val_t rc1 = MJS_UNDEFINED;
  mjs_exec( js, js_Math_funcs, &rc1 );

  // js_Math = js->mk_object();
  // js->set( global, "Math", ~0, js_Math );
  // js->set( js_Math, "sin", ~0, mjs_mk_foreign_func( js, (mjs_func_ptr_t) js_Math_sin ) );
}

int cmd_resetjs( int argc, const char * const * argv )
{
  resetjs();
  return 0;
}

const char js_Math_funcs[] =
  "let Math = {"
  "  ceil:      ffi('double ceil(double)'), "
  "  floor:     ffi('double floor(double)'), "
  "  round:     ffi('double round(double)'), "
  "  max:       ffi('double max(double,double)'), "
  "  min:       ffi('double min(double,double)'), "
  "  abs:       ffi('double abs(double)'), "
  "  sqrt:      ffi('double sqrt(double)'), "
  "  exp:       ffi('double exp(double)'), "
  "  log:       ffi('double log(double)'), "
  "  pow:       ffi('double pow(double, double)'), "
  "  sin:       ffi('double sin(double)'), "
  "  cos:       ffi('double cos(double)'), "
  "  rand:      ffi('int rand()'), "
  "  random: function() { return Math.rand() / 0x7fffffff; } "
  "};";


const Ffi_ptrs ffi_ptrs[] = {
  { "ceil",     FUN_D_D(ceil)  },
  { "floor",    FUN_D_D(floor) },
  { "round",    FUN_D_D(round) },
  { "max",      FUN_D_DD(fmax) },
  { "min",      FUN_D_DD(fmin) },
  { "abs",      FUN_D_D(fabs)  },
  { "sqrt",     FUN_D_D(sqrt)  },
  { "exp",      FUN_D_D(exp)   },
  { "log",      FUN_D_D(log)   },
  { "pow",      FUN_D_DD(pow)  },
  { "sin",      FUN_D_D(sin)   },
  { "cos",      FUN_D_D(cos)   },
  { "rand",     (void*)(rand)  },
  { nullptr,  nullptr  }
};

void *mjs_dlsym_local ( void * /*handle*/, const char *name )
{
  for( auto f = ffi_ptrs; f->name != nullptr; ++f ) {
    if( strcmp( name, f->name ) == 0 ) {
      return f->ptr;
    }
  }
  return nullptr;
}

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

