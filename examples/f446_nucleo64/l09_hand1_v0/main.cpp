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
DCL_CMD_REG( measure,  'M', "- measure all"  );


// auto out_nu_fmt = [](xfloat x) { return FltFmt(x, cvtff_auto,9,5); };


void idle_main_task()
{
  // leds[1].toggle();
}


uint32_t  tim_pwm_arr;


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );


int     t_pre       { 2000 };  // settle before measure
int     have_magn   {    0 };  // have magnetic encoder
int     stopsw      {    0 };  // data from stopswitches
int     l0_freq     { tim_pwm_freq };  // data from s
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
ADD_FOBJ( q0     );
ADD_FOBJ( q0_0   );
ADD_FOBJ( nu0    );

constexpr const NamedObj *const objs_info[] = {
  & ob_t_pre        ,
  & ob_have_magn    ,
  & ob_stopsw       ,
  & ob_l0_freq      ,
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
  UVAR_t =   10; // time step

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
  const uint32_t t_step = UVAR_t;

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

bool measure_all()
{
  stopsw = pins_l0_stop.readUint();
  if( ! have_magn ) {
    return false;
  }
  auto q0_i = - ang_sens.getAngleN_mDeg();
  q0  = q0_i / 1000.0f - q0_0;
  // TODO: really read AS5600
  // TODO: read from other coords: ADC
  return true;
}


// ------------------------------------------------------------ 

// vim: path=.,/usr/share/stm32cube/inc/,/usr/arm-none-eabi/include,/usr/share/stm32oxc/inc

