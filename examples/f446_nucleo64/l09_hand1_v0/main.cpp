#include <cstring>
#include <cerrno>
#include <cmath>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_statdata.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
#include <oxc_easing.h>
#include <oxc_as5600.h>

#include "main.h"

using namespace std;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "App to measure servo speed" NL;

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


auto out_v_fmt = [](xfloat x) { return FltFmt(x, cvtff_auto,6,3); };


void idle_main_task()
{
  // leds[1].toggle();
}


uint32_t  tim_pwm_arr;


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );


int     t_pre       {  500 };  // settle before measure
int     t_post      {  100 };  // settle after measure
int     t_meas      { 2000 };  // measure time
int     t_step      {   10 };  // time step
int     have_magn   {    0 };  // have magnetic encoder
int     stopsw      {    0 };  // data from stopswitches
int     l0_freq     { tim_pwm_freq };  // data from s
int     l0_freq_min {     10 };
int     l0_freq_max { 100000 };
int     l0_freq_n   {     10 };
int     l0_v_n      {     51 };
float   q0          {    0 };  // measured base coord
float   q0_0        {117.9f};  // zero point for q0
float   nu0         {    0 };  // measured base speed

#define ADD_IOBJ(x)    constexpr NamedInt   ob_##x { #x, &x }
#define ADD_FOBJ(x)    constexpr NamedFloat ob_##x { #x, &x }

// ADD_FOBJ( hx_a     );
ADD_IOBJ( t_pre    );
ADD_IOBJ( have_magn  );
ADD_IOBJ( stopsw  );
ADD_IOBJ( l0_freq  );
ADD_IOBJ( l0_freq_min  );
ADD_IOBJ( l0_freq_max  );
ADD_IOBJ( l0_freq_n  );
ADD_IOBJ( l0_v_n  );
ADD_FOBJ( q0     );
ADD_FOBJ( q0_0   );
ADD_FOBJ( nu0    );

constexpr const NamedObj *const objs_info[] = {
  & ob_t_pre        ,
  & ob_have_magn    ,
  & ob_stopsw       ,
  & ob_l0_freq      ,
  & ob_l0_freq_min  ,
  & ob_l0_freq_max  ,
  & ob_l0_freq_n    ,
  & ob_l0_v_n       ,
  & ob_q0           ,
  & ob_q0_0         ,
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

  MX_TIM_PWM_Init();

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
  const uint32_t n      = arg2ulong_d(   1, argc, argv, UVAR_n,    2, 10000 );
  const float   v0      = arg2xfloat_d(  2, argc, argv, 0.0f,  -1.0f,  1.0f );

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




int cmd_timinfo( int argc, const char * const * argv )
{
  tim_print_cfg( TIM_PWM );
  return 0;
}

int cmd_l0_freq( int argc, const char * const * argv )
{
  const uint32_t freq = arg2ulong_d(   1, argc, argv, 10000,  1, 1000000 );
  set_l0_freq( freq );
  tim_print_cfg( TIM_PWM );
  return 0;
}

int cmd_l0_pwm( int argc, const char * const * argv )
{
  const float pwm = arg2xfloat_d(   1, argc, argv, 0.0f,  0.0f, 1.0f );
  set_l0_pwm( pwm );
  return 0;
}

int cmd_l0_mode( int argc, const char * const * argv )
{
  const bool i1 = arg2bool_d(   1, argc, argv, false );
  const bool i2 = arg2bool_d(   2, argc, argv, false );
  set_l0_mode( i1, i2 );
  return 0;
}

int cmd_l0_v( int argc, const char * const * argv )
{
  const float v = arg2xfloat_d(   1, argc, argv, 0.0f,  -1.0f, 1.0f );
  set_l0_v( v );
  return 0;
}

int cmd_l0_scan( int argc, const char * const * argv )
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


int cmd_measure( int argc, const char * const * argv )
{
  auto ok = measure_all();
  std_out << stopsw << ' ' << q0 << ' ' << ok << NL;
  return 0;
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
  stopsw = pins_l0_stop.readUint();
  if( ! have_magn ) {
    return false;
  }
  auto q0_i = - ang_sens.getAngleN_mDeg();
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

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

