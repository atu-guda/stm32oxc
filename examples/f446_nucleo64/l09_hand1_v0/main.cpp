#include <cstring>
#include <cerrno>
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_atleave.h>
#include <oxc_statdata.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
#include <oxc_easing.h>
#include <oxc_as5600.h>

#include "main.h"

// using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to control hand1 (lab1)" NL;

const EasingFun easing_funcs[] {
  easing_one,         // 0
  easing_poly2_in,    // 1
  easing_poly2_out,   // 2
  easing_poly3_io,    // 3
  easing_trig_in,     // 4
  easing_trig_out,    // 5
  easing_trig_io,     // 6
  easing_step_01,     // 7
  easing_one,         // protect // 8
};
constexpr auto easing_funcs_n { std::size( easing_funcs) };

PinsOut pins_l0_ctrl( L0_Ctrl_Pin, 2 );
PinsIn  pins_l0_stop( L0_Stop_Pin, 1, GpioPull::down );

// --- local commands;
DCL_CMD_REG( test0,    'T', "n t_s t_e - test "  );
DCL_CMD_REG( timinfo,  'I', "- timers info"  );
DCL_CMD_REG( l0_v,     'Q', "[v] - set V on l0"  );
DCL_CMD_REG( l0_freq,  '\0', "Hz - set L0 PWM freq"  );
DCL_CMD_REG( l0_pwm,   '\0', "[pwm] - set pwm on l0"  );
DCL_CMD_REG( l0_mode,  'D', "i1 i2 - l0 mode"  );
DCL_CMD_REG( l0_scan,  'S', "scan l0 coord"  );
DCL_CMD_REG( measure,  'M', "- measure all"  );
DCL_CMD_REG( lab,      'L', "- do lab"  );
DCL_CMD_REG( go_0,     'Z', "q0 [v0] - to position"  );


auto out_v_fmt = [](xfloat x) { return FltFmt(x, cvtff_auto,7,3); };
auto out_q_fmt = [](xfloat x) { return FltFmt(x, cvtff_auto,7,3); };



uint32_t  tim_pwm_arr;


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );


uint32_t measure_tick      {   0 };
uint32_t measure_idle_step { 100 };

int     t_lab_max   { 100'000 };  // max lab time (ms)
int     t_pre       {     500 };  // settle before measure
int     t_post      {     100 };  // settle after measure
int     t_meas      {    2000 };  // measure time
int     t_step      {      10 };  // time step
int     have_magn   {       0 };  // have magnetic encoder
int     stopsw      {       0 };  // data from stopswitches
int     l0_freq     { tim_pwm_freq };  // data from s
int     l0_freq_min {      10 };
int     l0_freq_max {  100000 };
int     l0_freq_n   {      10 };
int     l0_v_n      {      51 };
int     q0_i        {       0 };  // measured base coord in ints
float   q0          {       0 };  // measured base coord
float   q0_g        {       0 };  // given base coord
float   q0_0        {    0.0f  };  // zero point for q0
float   q0_emax     {    1.0f  };  // good error
float   v0_def      {    0.5f  };  // default l0 speed in 0:1
float   v0_min      {    0.07f };  // minimal l0 speed
float   nu0         {    0.0f  };  // measured base speed

#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }

ADD_IOBJ( t_pre        );
ADD_IOBJ( have_magn    );
ADD_IOBJ( stopsw       );
ADD_IOBJ( l0_freq      );
ADD_IOBJ( l0_freq_min  );
ADD_IOBJ( l0_freq_max  );
ADD_IOBJ( l0_freq_n    );
ADD_IOBJ( l0_v_n       );
ADD_IOBJ( q0_i         );
ADD_FOBJ( q0           );
ADD_FOBJ( q0_g         );
ADD_FOBJ( q0_0         );
ADD_FOBJ( q0_emax      );
ADD_FOBJ( v0_def       );
ADD_FOBJ( v0_min       );
ADD_FOBJ( nu0          );

constexpr const NamedObj *const objs_info[] = {
  & ob_t_pre        ,
  & ob_have_magn    ,
  & ob_stopsw       ,
  & ob_l0_freq      ,
  & ob_l0_freq_min  ,
  & ob_l0_freq_max  ,
  & ob_l0_freq_n    ,
  & ob_l0_v_n       ,
  & ob_q0_i         ,
  & ob_q0           ,
  & ob_q0_g         ,
  & ob_q0_0         ,
  & ob_q0_emax      ,
  & ob_v0_def       ,
  & ob_v0_min       ,
  & ob_nu0          ,
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

void idle_main_task()
{
  if( HAL_GetTick() - measure_tick > measure_idle_step ) {
    measure_all();
  }
}



int main(void)
{
  BOARD_PROLOG;

  UVAR_n =  100; // default main loop count

  pins_l0_stop.initHW();
  pins_l0_ctrl.initHW();

  UVAR_v = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens;

  ang_sens.setCfg( AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off );
  if( ang_sens.isMagnetDetected() ) {
    have_magn = 1;
  }

  ang_sens.setStartPos( 2749 ); // TODO: ????????
  measure_all(); // to have correct values at start

  MX_TIM_PWM_Init();

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );


  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}


// TEST0
CMD_FUNCTION(test0)
{
  const uint32_t n      = arg2ulong_d(   1, argc, argv, UVAR_n,    2, 10000 );
  const float   v0      = arg2xfloat_d(  2, argc, argv, 0.0f,  -1.0f,  1.0f );

  ang_sens.setN_turn( 0 );
  HAL_TIM_PWM_Start( &htim_pwm, TIM_PWM_CHANNEL );

  uint32_t tm0 { HAL_GetTick() };
  const uint32_t tm00 { tm0 };

  set_l0_v( v0 );
  break_flag = 0;

  for( uint32_t i=0; i<n && !break_flag; ++i ) {
    const uint32_t tc { HAL_GetTick() - tm00 };
    measure_all();
    std_out << FmtInt( tc, 9 ) << ' ' << q0 << ' ' << stopsw;
    std_out << NL;
    delay_ms_until_brk( &tm0, t_step );
  }

  set_l0_v( 0 );

  return break_flag;
}




CMD_FUNCTION( timinfo )
{
  tim_print_cfg( TIM_PWM );
  return 0;
}

CMD_FUNCTION( l0_freq )
{
  const uint32_t freq = arg2ulong_d(   1, argc, argv, 10000,  1, 1000000 );
  set_l0_freq( freq );
  tim_print_cfg( TIM_PWM );
  return 0;
}

CMD_FUNCTION( l0_pwm )
{
  const float pwm = arg2xfloat_d(   1, argc, argv, 0.0f,  0.0f, 1.0f );
  set_l0_pwm( pwm );
  return 0;
}

CMD_FUNCTION( l0_mode )
{
  const bool i1 = arg2bool_d(   1, argc, argv, false );
  const bool i2 = arg2bool_d(   2, argc, argv, false );
  set_l0_mode( i1, i2 );
  return 0;
}

CMD_FUNCTION( l0_v )
{
  const float v = arg2xfloat_d(   1, argc, argv, 0.0f,  -1.0f, 1.0f );
  set_l0_v( v );
  return 0;
}

CMD_FUNCTION( l0_scan )
{
  const xfloat k_freq {( l0_freq_n > 1 ) ? (logxf( l0_freq_max / l0_freq_min ) / ( l0_freq_n - 1 ) ) : 0 };
  const xfloat k_v { ( l0_v_n >0 ) ?(2.0f / (l0_v_n-1)) : 0 };

  std_out << "# l0_freq   v     v_r     nu0 " NL;
  for( int i_f = 0; i_f < l0_freq_n && !break_flag; ++i_f ) {
    const float  freq_c = l0_freq_min * expxf( i_f * k_freq );
    set_l0_freq( freq_c ); // l0_freq may differ
    delay_ms( 1000 );
    set_l0_v( -1.0f ); // prepare from another direction
    delay_ms( 1000 );

    for( int i_v = 0; i_v < l0_v_n && !break_flag; ++i_v ) {
      float v = -1.0f + i_v * k_v;
      if( fabsf( v ) < 0.001f ) {
        v = 0;
      }
      if( !measure_speed( v ) ) {
        break;
      }
      const float v_r = get_l0_v();
      std_out << FmtInt(l0_freq,8) << ' ' << out_v_fmt(v) << ' ' << out_v_fmt(v_r) << ' ' << nu0 << NL;
    }
    std_out << NL;

  }

  set_l0_v( 0 );
  return 0;
}


CMD_FUNCTION( measure )
{
  auto ok = measure_all();
  std_out << stopsw << ' ' << q0 << ' ' << ok  << ' ' << q0_i << ' ' << ang_sens.getN_turn() << NL;
  return 0;
}

CMD_FUNCTION( lab )
{
  const int x_lab = arg2long_d(   1, argc, argv, 0 );
  // ang_sens.setN_turn( 0 );
  q0_g = 0;
  leds[0].reset();
  set_l0_v( 0 );
  DoAtLeave _( []() { set_l0_v( 0 ); } );
  HAL_TIM_PWM_Start( &htim_pwm, TIM_PWM_CHANNEL );

  auto rc  = lab_init( x_lab );
  if( rc > 1 ) {
    std_out << "# EMERG:"  NL;
    leds[0].set();
    set_l0_v( 0 );
  }
  if( rc > 0 ) {
    std_out << "# Error:" << rc  << NL;
    return rc;
  }

  uint32_t tm0 { HAL_GetTick() };
  const uint32_t tm00 { tm0 };

  break_flag = 0;
  uint32_t n = 1 + (uint32_t)t_lab_max / t_step;

  for( uint32_t i=0; i<n && !break_flag; ++i ) {
    const uint32_t tc { HAL_GetTick() - tm00 };
    measure_all();

    leds[1].set();
    rc = lab_step( tc );
    leds[1].reset();

    std_out << FmtInt( tc, 9 ) << ' ' << out_q_fmt(q0_g) << ' '  << out_q_fmt(q0)
      << ' ' << q0_i << ' ' << ang_sens.getN_turn()
      << ' ' << stopsw << ' ' << rc << NL;

    if( stopsw != 1 ) {
      leds[0].set();
      std_out << "# STOP!:" << rc  << ' ' << stopsw << NL;
      break_flag = 100;
      break;
    }

    if( rc > 0 ) {
      std_out << "# End:" << rc  << NL;
      break_flag = rc;
      break;
    }
    delay_ms_until_brk( &tm0, t_step );
  }

  return break_flag;
}

CMD_FUNCTION( go_0 )
{
  const float q0_e = arg2float_d(   1, argc, argv,   0.0f,  -90.00f,  90.0f  );
  const float k_v0 = arg2float_d(   2, argc, argv,   1.0f,    0.01f,  50.0f  );
  leds[0].reset();
  set_l0_v( 0 );
  DoAtLeave _( []() { set_l0_v( 0 ); } );
  HAL_TIM_PWM_Start( &htim_pwm, TIM_PWM_CHANNEL );

  uint32_t tm0 { HAL_GetTick() };
  const uint32_t tm00 { tm0 };

  int rc = 1;
  break_flag = 0;
  uint32_t n = 1 + (uint32_t)t_lab_max / t_step;

  for( uint32_t i=0; i<n && !break_flag; ++i ) {
    const uint32_t tc { HAL_GetTick() - tm00 };
    measure_all();

    leds[1].set();
    const float d_q0 = q0_e - q0;

    float v0 = std::clamp( d_q0 * k_v0 * 0.05f, -1.0f, 1.0f );
    if( fabsf( v0 ) < v0_min ) {
      v0 = v0_min * signf( v0 );
    }
    set_l0_v( v0 );
    leds[1].reset();

    std_out << FmtInt( tc, 9 ) << ' ' << out_q_fmt(q0_e) << ' '  << out_q_fmt(q0)
      << ' '  << out_q_fmt( d_q0 )
      << ' '  << out_q_fmt( v0 )
      << ' ' << stopsw << NL;

    if( stopsw != 1 ) {
      leds[0].set();
      std_out << "# STOP!: " << stopsw << NL;
      break_flag = 100;
      break;
    }
    if( fabsf( d_q0 ) < q0_emax ) {
      break;
    }

    delay_ms_until_brk( &tm0, t_step );
  }

  return rc * 1000 + break_flag;
}

// ------------------------------------------------------------ 



void set_l0_freq( uint32_t freq )
{
  TIM_PWM->PWM_CCR = 0;
  const int32_t in_freq = get_TIM_in_freq( TIM_PWM );
  // best prescaler value to maximaze ARR: need only for 16-bit timers
  const uint32_t psc = (uint32_t)( (uint64_t)in_freq / ((uint64_t)freq*65536) );
  TIM_PWM->PSC = psc;
  tim_pwm_arr = calc_TIM_arr_for_base_freq( TIM_PWM, freq );
  TIM_PWM->ARR = tim_pwm_arr;
  l0_freq = get_TIM_base_freq( TIM_PWM );
  UVAR_a = TIM_PWM->PSC;
  UVAR_b = tim_pwm_arr;
}

void set_l0_pwm( float pwm )
{
  const uint32_t arr = TIM_PWM->ARR;
  const uint32_t ccr = (uint32_t)( std::clamp( pwm, 0.0f, 1.0f ) * arr );
  TIM_PWM->PWM_CCR = ccr;
}

void set_l0_mode( bool i1, bool i2 )
{
  uint16_t v = ( i1 ? 1 : 0 ) | ( i2 ? 2 : 0 );
  pins_l0_ctrl.write( PinMask(v) );
}

void set_l0_v( float v )
{
  v  = std::clamp( v, -1.0f, 1.0f );
  const float va = std::fabsf( v );
  if( va < 1e-5f ) { // stop
    set_l0_mode( 0, 0 );
    set_l0_pwm( 0 );
    return;
  }

  if( v > 0 ) {
    set_l0_mode( 0, 1 );
  } else {
    set_l0_mode( 1, 0 );
  }
  set_l0_pwm( va );
}

float get_l0_v()
{
  auto md = pins_l0_ctrl.readUint();
  int k_dir = 0;
  switch( md ) {
    case 1: k_dir = -1; break;
    case 2: k_dir =  1; break;
    default: return 0;
  }
  if( TIM_PWM->ARR < 1 ) {
    return 0;
  }
  return k_dir * (float)(TIM_PWM->PWM_CCR) / TIM_PWM->ARR;
}

bool measure_all()
{
  leds[2].set();
  DoAtLeave _( []() { leds[2].reset(); } );
  measure_tick = HAL_GetTick();

  stopsw = pins_l0_stop.readUint();
  if( ! have_magn ) {
    leds[0].set();
    return false;
  }
  q0_i = - ang_sens.getAngleN_mDeg();
  q0  = q0_i / 1000.0f - q0_0;
  // TODO: read from other coords: ADC
  return true;
}

bool measure_speed( float v )
{
  const int t_end =  t_pre + t_meas;
  const int t_all =  t_end + t_post;

  float q0_s  {     0  }, q0_e {     0  };
  bool empty_q0_s { true }, empty_q0_e { true }; // flags: not measured
  leds[2].reset();
  break_flag = 0;
  set_l0_v( v );

  uint32_t tm0 { HAL_GetTick() };
  const uint32_t tm00 { tm0 };
  uint32_t t_s {0}, t_e {0}; // may be differ from t_pre, t_end

  for( int t = 0; t <= t_all && ! break_flag; t += t_step ) {
    const uint32_t tc { HAL_GetTick() - tm00 };
    if( !measure_all() ) {
      break_flag = 3;
      break;
    }
    if( empty_q0_s && t >= t_pre ) {
      q0_s = q0; empty_q0_s = false; leds[2].set();   t_s = tc;
    }
    if( empty_q0_e && t >= t_end ) {
      q0_e = q0; empty_q0_e = false; leds[2].reset(); t_e = tc;
    }
    delay_ms_until_brk( &tm0, t_step );
  }

  if( break_flag ) {
    set_l0_pwm( 0 );
    return false;
  }

  nu0 = 1000 * ( q0_e - q0_s ) / ( t_e - t_s ); // 1000 - ms/s

  return true;
}

// ------------------------------------------------------------ 

__weak int lab_init( int x )
{
  return 2;
}

__weak int lab_step( uint32_t tc )
{
  return 2;
}

// ------------------------------------------------------------ 

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

