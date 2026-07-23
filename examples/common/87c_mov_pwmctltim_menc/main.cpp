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

#include <board_robo_cfg.h>
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
  UX(                t_post,       500 ) \
  UX(                t_step,        20 ) \
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

ReturnCode init_hw_all();


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens_dev( i2cd );

// ------------------------ - local sensors ; ---------------------------------------

LinearCoordTransform coo_tr_AS5600( -SensorAS5600::k_i2ph, -2.1015536f );
SensorAS5600 ang_sens_ph( "q0_sens", ang_sens_dev );
SensorBase   q0_ang_sens( ang_sens_ph, 0, coo_tr_AS5600 );

// ------------------------ - local sensors end ---------------------------------------


TIM_HandleTypeDef tim_pwm_h;

constinit PwmCtlTim pwm1( TIM_MPWM_BASE, tim_MPWM_chspins, tim_pwm_h );
RoboPwmCtl pwm1_ctl( "pwm1_ctl", pwm1 );

PinGpio pwm_left_pin{  MPWM_CtlPin_L  };
PinGpio pwm_right_pin{ MPWM_CtlPin_R };
RoboPin q0_pin_l{ "q0_pin_l", pwm_left_pin };
RoboPin q0_pin_r{ "q0_pin_r", pwm_right_pin };
LinearCoordTransform q0_coord_tr { 1.986f, 0 }; // TODO: coeff (mech dependent) to header
ActuDcPwm_1P2D q0_actu( pwm1_ctl, 0, q0_pin_l, q0_pin_r, q0_coord_tr );

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



// misc tests
// LinearCoordTransform toPh( 0.01f, 1.0f );
// LinearCoordTransform toIn( 0.02f, 2.0f, LinearCoordTransform::PhysicalToInternalInit{} );

RoboDevice* hw_robo_devs[] {
  &q0_pin_l,
  &q0_pin_r,
  &pwm1_ctl,
  &ang_sens_ph,
};

RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( hw_robo_devs, robo_joints );

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
    robo.measure_all();
  }
}


int main(void)
{
  BOARD_PROLOG;

  UVAR_l =    1; // idLe after run ?

  print_var_hook = print_var_ex;
  set_var_hook   = set_var_ex;

  if( ! init_hw_all().isOk() ) {
    std_out << "# Error: HW init" << NL;
    die4led( 1_mask );
  };

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  start_count_time();

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  // q0 sens:
  i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens_dev;

  // q0 PWM:
  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq( TIM_MPWM_BASE ), 20000 );
  pwm1.setAllowPSCadj( true );
  tim_pwm_h.Instance = TIM_MPWM;
  pwm1.setHardParams( psc_i, arr_i, TIM_COUNTERMODE_UP );
  pwm1.enable();

  return robo.init_all();
}

// TODO: move to robo
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

  first_measure = 0;
  return rcOk;
}


CMD_FUNCTION( test0 )
{
  float pwm_v = arg2float_d( 1, argc, argv, 0.5f, 0.0f, 1.0f );
  int v0 = arg2long_d( 2, argc, argv,  UVAR_v, INT_MIN, INT_MAX );

  pwm1.setPwm( 0, pwm_v );

  pwm_left_pin.write(  v0 & 1 );
  pwm_right_pin.write( v0 & 2 );
  return v0;
}

CMD_FUNCTION( tinfo ) // P
{
  tim_print_cfg( TIM_MPWM_BASE );

  std_out << "# freq:  "  << pwm1.getFreq() << NL;

  dump32( (void*)TIM_MPWM_BASE, 0x60 );

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

  tim_print_cfg( TIM_MPWM_BASE );

  return 0;
}

struct Data_setV
{
  float v;
};

ReturnCode run_v_loop( const RunLoopState &rls, const RunLoopData &rld, void *data )
{
  if( !data ) {
    return rcFatal;
  }
  auto d = static_cast<Data_setV*>(data);

  calc_current_time();
  auto rc = robo.measure_all();
  if( rc.isError() ) {
    return rc;
  }
  float v = ( ( rls.stage & RunLoopState::stage_num_mask ) == 1 ) ? d->v : 0;

  if( rls.stage & RunLoopState::stage_change_flag  ) {
    q0_actu.setV( v );
    robo.commit_all();
  }

  std_out << FmtInt( rls.tc, 8 ) << ' '  << v << ' ' << FmtInt( pwm1.getPwmRaw( 0 ), 6 )
    << ' ' << q0_actu.get_v_int() << ' ' << q0_actu.get_v_phy()
    << ' ' <<  FmtInt( ang_sens_ph.get(0), 6 )  << ' ' << r2d( q0_ang_sens.get() )
    << NL;

  return rcOk;
}


CMD_FUNCTION( setV ) // V
{
  struct Data_setV d;
  d.v          = arg2float_d( 1, argc, argv, 0 );
  auto t_run_c = arg2ulong_d( 2, argc, argv, t_run, 0 );

  RunLoopData rld( t_step, t_pre, t_run_c , t_post );

  start_count_time();
  auto rc = run_periodic( rld, run_v_loop, &d );

  if( UVAR_l ) {
    q0_actu.idle();
    robo.commit_all();
  }

  return rc.isOk() ? 0 : 2;
}

// std_out << FltFmt( t_cur_f, cvtff_fix, 9, 3 ) << ' ' << v_c << ' '
//         << FmtInt( q0_i, 8 ) << ' ' << q0 << ' ' << r2d( q0 ) << ' '
//         << nu0_i << NL;


CMD_FUNCTION( measure ) // M
{
  std_out << ang_sens_dev.getAngleN() << ' ' << ang_sens_dev.isMagnetDetected() << ' '
          << q0_ang_sens.get() << ' ' << q0_ang_sens.get_i() << ' ' << r2d( q0_ang_sens.get() )
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

  float v   = arg2float_d( 1, argc, argv, q0_ang_sens.get() );
  coo_tr_AS5600 = LinearCoordTransform( -SensorAS5600::k_i2ph, coo_tr_AS5600.b-v );
  cmd_measure( argc, argv );

  return 0;
}








// -----------------------  timers init part ---------------

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_MPWM ) {
    TIM_MPWM_CLKEN();
    return;
  }
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_MPWM ) {
    TIM_MPWM_CLKDIS();
    return;
  }
}

