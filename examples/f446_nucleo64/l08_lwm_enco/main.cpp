#include <cstring>
#include <cerrno>
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_hx711.h>
#include <oxc_statdata.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>

#include "main.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure fb-less serwo speed" NL;


// --- local commands;
DCL_CMD_REG( test0,    'T', "n t_s t_e - test "  );
DCL_CMD_REG( timinfo,  'I', "- timers info"  );
DCL_CMD_REG( set_t_on, 'L', "us - set t_on"  );
DCL_CMD_REG( meas1,    'M', "t_on_us - one measure"  );
DCL_CMD_REG( angle,    'A', "[set_val] - measure and ?set angle in ticks"  );
DCL_CMD_REG( measF,    'F', "[n] [set_0] [off] - measure force"  );


auto out_nu_fmt = [](xfloat x) { return FltFmt(x, cvtff_auto,9,5); };


void idle_main_task()
{
  // leds[1].toggle();
}


void set_t_on_from_us( uint32_t t_on_us );
uint32_t  go_measure( uint32_t t_on_us, int32_t t_pre, int32_t t_meas, int32_t t_post );
uint32_t  tim_lwm_arr;

int start_measure_times();
int stop_measure_times();
void out_times( int se ); // bits: 1: start label, 2: NL
ReturnCode do_off();

constexpr auto hx_mode = HX711::HX711_mode::mode_A_x128;
HX711 hx711( HX711_EXA_SCK_PIN, HX711_EXA_DAT_PIN );
xfloat hx_a { 5.0617948e-07f };
xfloat hx_b {  -0.03399918f };
StatChannel st_f;
uint32_t measure_f( int n );

xfloat  nu_1    {   0  };

int     t_pre   { 2000 };  // settle before measure
int     t_post  { 1000 };
int     t_s_def {  500 };  // default start LWM us value
int     t_e_def { 2500 };  // default stop  LWM us value
int     n_me_f  {   40 };  // number of the force measurements
int     cnt_1   {    0 };  // first counter
int     enco1_n {  600 };  // pulses per turn for encoder
int     tick_0  {    0 };  // start tick
int     dlt_t   {    0 };  // ticks delta

#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }

ADD_FOBJ( hx_a     );
ADD_FOBJ( hx_b     );
ADD_FOBJ( nu_1     );
ADD_IOBJ( t_pre    );
ADD_IOBJ( t_post   );
ADD_IOBJ( t_s_def  );
ADD_IOBJ( t_e_def  );
ADD_IOBJ( n_me_f   );
ADD_IOBJ( cnt_1    );
ADD_IOBJ( enco1_n  );
ADD_IOBJ( tick_0   );
ADD_IOBJ( dlt_t    );

constexpr const NamedObj *const objs_info[] = {
  & ob_hx_a     ,
  & ob_hx_b     ,
  & ob_nu_1     ,
  & ob_t_pre    ,
  & ob_t_post   ,
  & ob_t_s_def  ,
  & ob_t_e_def  ,
  & ob_n_me_f   ,
  & ob_cnt_1    ,
  & ob_enco1_n  ,
  & ob_tick_0   ,
  & ob_dlt_t    ,
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

  UVAR_n =  100; // default main loop count

  hx711.initHW();
  MX_TIM_CNT_Init();
  HAL_TIM_Base_Start( &htim_cnt );
  MX_TIM_LWM_Init();
  HAL_TIM_PWM_Start( &htim_lwm, TIM_LWM_CHANNEL );

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


// TEST0
int cmd_test0( int argc, const char * const * argv )
{
  int n        = arg2long_d(   1, argc, argv, UVAR_n,  2, 10000 );
  int t_s      = arg2long_d(   2, argc, argv, t_s_def, 1, 10000 );
  int t_e      = arg2long_d(   3, argc, argv, t_e_def, 2, 10000 );

  int t_dlt = ( t_e - t_s ) / (n-1);


  std_out <<  "# Test0: n= " << n << " n_me_f= " << n_me_f << " t_pre= " << t_pre
          << " t_s= " << t_s << " t_e= " << t_e << " t_dlt " << t_dlt << NL;
            // 500 3827   3600   908  000.42037 0.02557362
  std_out << "#t_on dnf   dlt_t cnt_1  nu_1       F" NL;

  break_flag = 0;


  for( int i=0; i<n && !break_flag; ++i ) {
    uint32_t t_on = t_s + i * t_dlt;
    int32_t dnf = go_measure( t_on, t_pre, n_me_f, t_post );
    std_out << FmtInt( t_on, 5 ) << ' ' << FmtInt( dnf, 3 ) <<  ' ';
    out_times( 0 );
    std_out << ' ' << st_f.mean << NL;
    if( dnf < 1 ) {
      break;
    }
  }

  set_t_on_from_us( 0 );

  return 0;
}

int cmd_meas1( int argc, const char * const * argv )
{
  int32_t t_on     = arg2long_d(   1, argc, argv,      1500,   0, 10000 );

  std_out <<  "# meas1: n_me_f= " << n_me_f << " t_pre= " << t_pre << " t_post " << t_post << NL;

  int32_t dn = go_measure( t_on, t_pre, n_me_f, t_post );
  std_out << t_on << ' ' << dn << NL;

  return 0;
}

// TMP: unused for now
int cmd_angle( int argc, const char * const * argv )
{
  int x    = arg2long_d(   1, argc, argv, -1 );
  int n    = arg2long_d(   2, argc, argv,  4, 1, 20 );

  std_out <<  "# x= " <<  x << " n= " << n << " xf= |" << FmtInt( x, n ) << NL;

  return 0;
}

uint32_t go_measure( uint32_t t_on_us, int32_t t_pre, int32_t n_me_f, int32_t t_post )
{
  leds[0] = 1;
  set_t_on_from_us( t_on_us );

  if( delay_ms_brk( t_pre ) ) { // settle
    return 0;
  }
  leds[0] = 0;

  leds[1] = 1;
  start_measure_times();
  uint32_t nmf = measure_f( n_me_f );
  if( nmf < 1 ) {
    return 0;
  }
  stop_measure_times();
  leds[1] = 0;

  if( t_post >= 0 ) {
    set_t_on_from_us( 0 );
    delay_ms_brk( t_post );
  }

  return nmf;
}


void set_t_on_from_us( uint32_t t_on_us )
{
  const uint32_t ccr = (uint32_t)( t_on_us * 1000L / tim_lwm_dt_ns ); // really *2, but for outher freqs...
  TIM_LWM->LWM_CCR = ccr;
}

int cmd_set_t_on( int argc, const char * const * argv )
{
  uint32_t t_on_us = arg2long_d(   1, argc, argv, 1500,  0, 3000 );
  set_t_on_from_us( t_on_us );

  std_out << "# LWM: " << t_on_us << " us, ccr= " << TIM_LWM->LWM_CCR << NL;

  return 0;
}

int cmd_measF( int argc, const char * const * argv )
{
  int n         = arg2long_d( 1, argc, argv, n_me_f, 1, 10000 );
  int set_zero  = arg2long_d( 2, argc, argv,      0, 0,     1 );
  int off_after = arg2long_d( 3, argc, argv,      0, 0,     1 );

  start_measure_times();
  measure_f( n );
  stop_measure_times();

  if( off_after ) {
    do_off();
  }

  std_out << "# force: " << st_f.mean << ' ' << st_f.n << ' ' << st_f.sd;
  out_times( 2 );
  if( set_zero ) {
    hx_b -= st_f.mean;
  }
  return 0;
}


uint32_t measure_f( int n )
{
  st_f.reset();
  hx711.read( hx_mode );
  break_flag = 0;
  for( int i=0; i<n && !break_flag; ++i ) {
    auto fo_ret = hx711.read( hx_mode );
    if( fo_ret.isOk() ) {
      const auto fo_i = fo_ret.v;
      xfloat fo = fo_i * hx_a + hx_b;
      st_f.add( fo );
    }
  }
  st_f.calc();
  return st_f.n;
}


int start_measure_times()
{
  tick_0 = GET_OS_TICK();
  TIM_CNT->CNT  = 0;
  return tick_0;
}

int stop_measure_times()
{
  const TickType t1 = GET_OS_TICK();
  cnt_1 = TIM_CNT->CNT;

  dlt_t = t1 - tick_0;
  const xfloat den1 = dlt_t * enco1_n;
  nu_1 = ( den1 != 0 ) ? ( -1.0e+3f * cnt_1 / den1 ) : 0; // '-' - direction

  return dlt_t;
}

void out_times( int se )
{
  if( se & 1 ) {
    std_out << "# times: ";
  };
  std_out
          << ' ' << FmtInt( dlt_t, 5 )
          << ' ' << cnt_1 // FmtInt( cnt_1, 5 )
          << ' ' << out_nu_fmt( nu_1 );
  if( se & 2 ) {
    std_out << NL;
  };
}

ReturnCode do_off()
{
  set_t_on_from_us( 0 );
  delay_ms( 200 );
  return rcOk;
}


int cmd_timinfo( int argc, const char * const * argv )
{
  tim_print_cfg( TIM_CNT );
  tim_print_cfg( TIM_LWM );
  return 0;
}

// ------------------------------------------------------------ 



// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

