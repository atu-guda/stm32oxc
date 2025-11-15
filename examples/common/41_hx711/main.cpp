#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_floatfun.h>
#include <oxc_statdata.h>
#include <oxc_namedfloats.h>

#include <oxc_hx711.h>

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to test HX711 ADC" NL;

// --- local commands;
DCL_CMD_REG( test0, 'T', "[n] - test HX711 ADC"  );


HX711 hx711( HX711_SCK_GPIO, HX711_SCK_PIN, HX711_DAT_GPIO, HX711_DAT_PIN );
// -0.032854652221894 5.06179479849053e-07
xfloat hx_a =  5.0617948e-07f;
xfloat hx_b =  -0.032854f;

#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }
ADD_FOBJ( hx_a  );
ADD_FOBJ( hx_b  );

constexpr const NamedObj *const objs_info[] = {
  & ob_hx_a,
  & ob_hx_b,
  nullptr
};

NamedObjs objs( objs_info );

// print/set hook functions

bool print_var_ex( const char *nm, int fmt )
{
  return objs.print( nm, fmt );
}

bool set_var_ex( const char *nm, const char *s )
{
  auto ok =  objs.set( nm, s );
  print_var_ex( nm, 0 );
  return ok;
}

int main(void)
{
  BOARD_PROLOG;

  UVAR('t') = 100; // 100 ms
  UVAR('n') = 20;
  UVAR('l') = 1; // indicate by LED
  UVAR('m') = 0; // mode 0-2

  hx711.initHW();

  BOARD_POST_INIT_BLINK;

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  pr( NL "##################### " PROJ_NAME NL );

  srl.re_ps();

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, nullptr );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  uint32_t t_step = UVAR('t');
  uint32_t n = arg2long_d( 1, argc, argv, UVAR('n'), 1, 1000000 ); // number of series

  std_out <<  NL "# Test0: n= " <<  n <<  " t= " <<  t_step <<  NL;

  xfloat vf;
  StatData sdat( 1 );

  leds.set(   BIT0 | BIT1 | BIT2 ); delay_ms( 100 );
  leds.reset( BIT0 | BIT1 | BIT2 );

  uint32_t tm0, tm00;
  int rc = 0;
  bool do_out = ! UVAR('b');
  delay_ms( t_step );

  break_flag = 0;
  for( decltype(n) i=0; i<n && !break_flag; ++i ) {

    uint32_t tcc = HAL_GetTick();
    if( i == 0 ) {
      tm0 = tcc; tm00 = tm0;
    }

    if( UVAR('l') ) {  leds.set( BIT1 ); }
    auto v = hx711.read( HX711::HX711_mode( UVAR('m') & 3 ) );
    if( UVAR('l') ) {  leds.reset( BIT1 ); }


    int dt = tcc - tm00; // ms
    if( do_out ) {
      std_out <<  FltFmt(   0.001f * dt, cvtff_auto, 12, 4 );
    }

    vf = (xfloat)(v.v);
    if( do_out ) {
      std_out  << ' '  <<  v.rc <<  ' ' << HexInt(v.v) << ' ' <<  v.v     << NL;
      if( v.isOk() ) {
        sdat.add ( &vf );
      }
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  sdat.calc();
  std_out << sdat << NL;
  std_out << "m = " << ( sdat.d[0].mean * hx_a + hx_b ) << NL;

  return rc;
}


// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

