#include <climits>
#include <oxc_auto.h>
#include <oxc_atleave.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
#include <oxc_main.h>
#include <oxc_pingpio.h>
#include <oxc_robopin.h>

#include <oxc_easing.h>
#include <oxc_pwmctltim.h>
#include <oxc_robopwmctl.h>
#include <oxc_actu_dcpwm.h>

#include <oxc_sensor_as5600.h>

#include "main.h"

using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test PWM motor + AS5600 encoder" NL;

// ------------------------ named global vars -----------------------------------

#define NAMED_VARS_LIST    \
  IX(                 debug,         0 ) \
  IX(                  q0_i,         0 ) \
  UX(  off_motor_idle_ticks,     60000 ) \
  UX(    measure_idle_ticks,       100 ) \
  UX(                t_pre,        200 ) \
  UX(                t_run,       1000 ) \
  UX(                t_post,      2000 ) \
  UX(                t_step,        10 ) \
  UX(             t_start_i,         0 ) \
  UX(               t_cur_i,         0 ) \
  UX(                  t_dt,         0 ) \
  UX(         first_measure,         1 ) \
  FX(                t_dt_f,      0.0f ) \
  FX(               t_cur_f,      0.0f ) \
  FX(                    q0,      0.0f ) \
  FX(                 nu0_i,      0.0f )

uint32_t last_cmd_end_tick     {     0 };
uint32_t last_measure_tick     {     0 };


// global vars itself
#define IX( name, init)      int   name { init };
#define UX( name, init) uint32_t   name { init };
#define FX( name, init)    float   name { init };
NAMED_VARS_LIST
#undef IX
#undef UX
#undef FX

// objects to global vars
#define IX(name, init) constexpr NamedInt   ob_##name { #name, &name };
#define UX(name, init) constexpr NamedInt   ob_##name { #name, &name };
#define FX(name, init) constexpr NamedFloat ob_##name { #name, &name };
NAMED_VARS_LIST
#undef IX
#undef UX
#undef FX

#define IX(name, init) &ob_##name,
#define UX(name, init) &ob_##name,
#define FX(name, init) &ob_##name,
constexpr const NamedObj *const objs_info[] = {
    NAMED_VARS_LIST
    nullptr
};
#undef IX
#undef UX
#undef FX

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

// ------------------------ end: named global vars -----------------------------------


// ------------------------ - local commands; ---------------------------------------
DCL_CMD_REG(      test0,  'T',     " [arg ] - test something"  );
DCL_CMD_REG(      tinfo,  'P',     " print info"  );
DCL_CMD_REG(    setfreq,  'F',     " Hz - set freq"  );
DCL_CMD_REG(      pulse,  'U',     " []- test pulse in us"  );
DCL_CMD_REG(       setV,  'V',     " v [t_us] - set v"  );
DCL_CMD_REG(    measure,  'M',     " - measure angle..."  );
DCL_CMD_REG(    zero_q0, '\0',     "[val] - zero q0 (to val or current)"  );

// -------------------------------------------------------------------------------------

ReturnCode measure_all();


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens_dev( i2cd );

// ------------------------ - local sensors ; ---------------------------------------

LinearCoordTransform coo_tr_AS5600( -SensorAS5600::k_i2ph, -2.1015536f );
SensorAS5600 ang_sens_ph( ang_sens_dev );
SensorBase   ang_sens_q0( ang_sens_ph, 0, coo_tr_AS5600 );

// ------------------------ - local sensors end ---------------------------------------

void init_mot0();
bool commit_all();

TIM_HandleTypeDef tim_pwm_h;
// auto tim_pwm_clk_enable = [](){ TIM_PWM_CLKEN }; // just test: how to store code from macro

constinit PwmCtlTim pwm1( TIM_PWM_BASE, tim_pwm_chspins, tim_pwm_h );
RoboPwmCtl q0_pwm( "q0_pwm", pwm1 );

PinGpio pwm_left_pin{  PwmLeftPin  };
PinGpio pwm_right_pin{ PwmRightPin };
RoboPin q0_pin_l{ "q0_pin_l", pwm_left_pin };
RoboPin q0_pin_r{ "q0_pin_r", pwm_right_pin };
LinearCoordTransform q0_coord_tr { 1.986f, 0 }; // TODO: coeff (mech dependent) to header
ActuDcPwm_1P2D q0_actu( q0_pwm, 0, q0_pin_l, q0_pin_r, q0_coord_tr );

const EasingFunInfo easing_fun_info[] = {
  { easing_one,       1.0f },    // 0
  { easing_poly2_in,  0.5f },    // 1
  { easing_poly2_out, 0.5f },    // 2
  { easing_poly3_io,  0.6f },    // 3
  { easing_trig_in,   0.6f },    // 4 0.6 approx 1/pi_half_f
  { easing_trig_out,  0.6f },    // 5
  { easing_trig_io,   0.6f },    // 6
  { easing_step_01,   1.0f },    // 7
  { easing_one,       1.0f }     // protect // 8
};
constexpr auto easing_fun_n { std::size( easing_fun_info) };


void start_count_time()
{
  t_start_i = GET_OS_TICK();
  t_cur_i = 0;  t_cur_f = 0;
  first_measure = 1;
}


void calc_current_time()
{
  t_cur_i = GET_OS_TICK() - t_start_i;
  t_cur_f = t_cur_i * 1e-3f;
}

void idle_main_task()
{
  calc_current_time();
  if( t_cur_i - last_cmd_end_tick >= off_motor_idle_ticks ) {
    // stop();
    last_cmd_end_tick = t_cur_i;
  }
  if( ( t_cur_i - last_measure_tick ) >= measure_idle_ticks ) {
    measure_all();
  }
}

// misc tests
// LinearCoordTransform toPh( 0.01f, 1.0f );
// LinearCoordTransform toIn( 0.02f, 2.0f, LinearCoordTransform::PhysicalToInternalInit{} );

RoboDevice* hw_robo_devices[] {
  &q0_pin_l,
  &q0_pin_r,
  &q0_pwm,
};

int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 100;
  UVAR_n =  20;

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens_dev;

  if( ang_sens_ph.initHW() != rcOk  ) {
    std_out << "# Error: no magnet sensor" << NL;
    die4led( 1_mask );
  }

  init_mot0();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  start_count_time();

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

void init_mot0()
{
  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq( TIM_PWM ), 20000 );
  pwm1.setAllowPSCadj( true );
  tim_pwm_h.Instance = TIM_PWM;
  pwm1.setHardParams( psc_i, arr_i );
  q0_pwm.initHW();
  for( auto dev : hw_robo_devices ) {
    dev->initHW();
  }
  pwm1.enable();
}

CMD_FUNCTION( test0 )
{
  float pwm_v = arg2float_d( 1, argc, argv, 0.5f, 0.0f, 1.0f );
  int v0 = arg2long_d( 2, argc, argv,  UVAR_v, INT_MIN, INT_MAX );

  pwm1.setPwm( 0, pwm_v );

  pwm_left_pin.write(  v0 & 1 );
  pwm_right_pin.write( v0 & 2);
  return v0;
}

CMD_FUNCTION( tinfo ) // P
{
  tim_print_cfg( TIM_PWM );

  std_out << "# freq:  "  << pwm1.getFreq() << NL;

  dump32( TIM_PWM, 0x60 );

  return 0;
}

CMD_FUNCTION( setfreq ) // F
{
  auto freq = arg2float_d( 1, argc, argv, 1, 0.01f );

  pwm1.setFreq( freq );

  std_out << "# freq: " << freq << " => " << pwm1.getFreq() << NL;

  return 0;
}

CMD_FUNCTION( pulse ) // U
{
  uint32_t pu = arg2ulong_d( 1, argc, argv, 0 );
  pwm1.setPulse( 0, pu );
  std_out << '#' << pu << ' ' << pwm1.getPwmRaw( 0 ) << NL;

  tim_print_cfg( TIM_PWM );

  return 0;
}

CMD_FUNCTION( setV ) // V
{
  float v   = arg2float_d( 1, argc, argv, 0 );
  auto  t_r = arg2ulong_d( 2, argc, argv, t_run, 1 );

  const auto t_all { t_pre + t_r + t_post };
  const auto t_e1  { t_pre + t_r          }; // end of active phase


  start_count_time();
  uint32_t tc0 { GET_OS_TICK() };
  break_flag = 0;
  int state = 0;
  float v_c = 0;
  for( decltype(+t_all) t=0; t <= t_all && !break_flag; t += t_step ) {
    calc_current_time();
    measure_all();

    if( state == 0 && t_cur_i >= t_pre ) {
      q0_actu.setV( v ); v_c = v;
      state = 1;
    }
    if( state == 1 && t_cur_i >= t_e1 ) {
      q0_actu.setV( 0 ); v_c = 0;
      state = 2;
    }

    std_out << FltFmt( t_cur_f, cvtff_fix, 9, 3 ) << ' ' << v_c << ' '
            << FmtInt( q0_i, 8 ) << ' ' << q0 << ' ' << r2d( q0 ) << ' '
            << nu0_i << NL;

    delay_ms_until_brk( &tc0, t_step );
  }
  // std_out << '#' << pu << ' ' << pwm1.getPwmRaw( 0 ) << NL;

  return 0;
}


CMD_FUNCTION( measure ) // M
{
  std_out << ang_sens_dev.getAngleN() << ' ' << ang_sens_dev.isMagnetDetected() << ' '
          << ang_sens_q0.get() << ' ' << ang_sens_q0.get_i() << ' ' << r2d( ang_sens_q0.get() )
          << NL;

  return 0;
}


CMD_FUNCTION( zero_q0 )
{
  auto rc = ang_sens_ph.measure();
  if( rc.isError() ) {
    std_out << "# Measure error " << rc.data << NL;
    return 1;
  }

  float v   = arg2float_d( 1, argc, argv, ang_sens_q0.get() );
  coo_tr_AS5600 = LinearCoordTransform( -SensorAS5600::k_i2ph, coo_tr_AS5600.b-v );
  cmd_measure( argc, argv );

  return 0;
}






void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_PWM ) {
    TIM_PWM_CLKEN;
    return;
  }
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_PWM ) {
    TIM_PWM_CLKDIS;
    return;
  }
}

ReturnCode measure_all()
{
  auto old_tick { last_measure_tick };
  last_measure_tick = t_cur_i;
  if( first_measure ) {
    t_dt = 0;
    t_dt_f = 0;
  } else {
    t_dt = std::max( last_measure_tick - old_tick, 1_u32 );
    t_dt_f = t_dt * 1e-3f;
  }

  leds[2].set();
  DoAtLeave _( []() { leds[2].reset(); } );

  auto rc = ang_sens_ph.measure();
  if( !rc.isOk() ) {
    leds[0].set();
    return rc;
  }

  auto q0_old = q0;
  q0_i  = ang_sens_q0.get_i(); // debug
  q0  = ang_sens_q0.get();
  nu0_i = first_measure ? 0 : ( ( q0 - q0_old ) / t_dt_f );

  // for( auto ps : sensors ) {
  //   auto rc = ps->measure( adc_n );
  //   if( rc < 1 ) {
  //     ledsx[0].set();
  //     return 0;
  //   }
  // }
  //

  first_measure = 0;
  return rcOk;
}

