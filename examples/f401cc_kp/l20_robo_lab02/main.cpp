#include <cstring>
#include <ranges>
#include <algorithm>

#include <oxc_auto.h>
#include <oxc_main.h>
#include <oxc_atleave.h>
#include <oxc_floatfun.h>
#include <oxc_namedints.h>
#include <oxc_namedfloats.h>
#include <oxc_outstr.h>
#include <oxc_easing.h>

#include <oxc_robo_base.h>

#include <oxc_gpio_pin_d.h>
#include <oxc_gpio_pins_d.h>

#include <oxc_tim_pwm_d.h>

#include <oxc_adcint_adc_d.h>

#include <board_robo_cfg.h>
#include "main.h"


using namespace oxc;
using namespace SMLRL;
namespace ranges = std::ranges;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

USBCDC_CONSOLE_DEFINES;

// int adc_n        {  100 };

// ------------------------ named global vars -----------------------------------

#define NAMED_VARS_LIST    \
  IX(                 debug,         0 ) \
  IX(               dry_run,         0 ) \
  UX(  off_motor_idle_ticks,     60000 ) \
  UX(                t_pre,        200 ) \
  UX(                t_run_c,   100000 ) \
  UX(                t_post,       500 ) \
  UX(                t_step,        20 ) \
  UX(                adc_n,         20 ) \
  FX(                a_0,         1.0f ) \
  FX(                a_1,         1.0f ) \
  FX(                T_0,         2.0f ) \
  FX(                T_1,         2.2f )


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
DEFINE_NAMEDOBJ_FUNCS(objs)


// ------------------------ end: named global vars -----------------------------------

// auto out_q_fmt = [](xfloat x) { return FltFmt(x, cvtff_fix,8,4); };



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


TIM_HandleTypeDef  tim_servolwm_h;
Tim_Pwm_Dev        servolwm_hd( TIM_SERVOLWM_BASE, tim_SERVOLWM_chspins, tim_servolwm_h, 0xFFFF );
float              servolwm_io[3*6*3]; // TODO???
PwmRoboCapability  servolwm_rd( servolwm_hd, 3, servolwm_io, 600 );

TransFFLinLim tro_rad2pulse( 2.0e-3f/pi_f, 1.5e-3f, 0.5e-3f, 2.5e-3f ); // angle +/ pi/2 -> 500-2500 μs
OutChFF    oc_servolwm_rad2pulse0( servolwm_rd, PwmRoboCapability::ch0_pulse  , tro_rad2pulse );
OutChFF    oc_servolwm_rad2pulse1( servolwm_rd, PwmRoboCapability::ch0_pulse+1, tro_rad2pulse );
OutChFF    oc_servolwm_rad2pulse2( servolwm_rd, PwmRoboCapability::ch0_pulse+2, tro_rad2pulse );

IoRoboCapability* rcaps[] {
  &servolwm_rd,      // 0
};

RoboObject* robo_objs[] {
  &servolwm_rd,      // 0
};

OutChFBase* outchfs[] {
  &oc_servolwm_rad2pulse0, // 0
  &oc_servolwm_rad2pulse1, // 1
  &oc_servolwm_rad2pulse2, // 2
};

RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( robo_objs, robo_joints );

ReturnCode init_hw_all();

const char* common_help_string = "robo_lab02_ " __DATE__ " " __TIME__ NL;

// --- local commands;

// ------------------------   commands
DCL_CMD_REG( test0,       'T', " alp_0 alp_1 - test " );
DCL_CMD_REG( run0,        'R', " - run " );
DCL_CMD_REG( prt,         '\0', " - print timer info " );


void idle_main_task()
{
  robo.at_main_idle();
  // leds[1].toggle();
}

void on_sigint( int /* c */ )
{
  // tim_lwm_stop();
  break_flag = 1;
}

int main(void)
{
  STD_PROLOG_USBCDC;

  UVAR_t =    50;
  UVAR_n =    1000;

  SET_NAMEDOBJ_FUNCS;

  leds.initHW();   leds.reset( 0xFF_mask );

  if( ! init_hw_all().isOk() ) {
    std_out << "# Error: HW init" << NL;
    die4led( 1_mask );
  };

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );
  dev_console.setOnSigInt( on_sigint );
  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  TIM_SERVOLWM_CLKEN();
  servolwm_hd.initHW();
  servolwm_hd.setFreq( SERVOLWM_FREQ );
  servolwm_hd.enable();
  return robo.init_all();
}




CMD_FUNCTION( test0 )
{
  float alp0  = arg2float_d( 1, argc, argv, 0,  -pi_f/2, pi_f/2 );
  float alp1  = arg2float_d( 2, argc, argv, 0,  -pi_f/2, pi_f/2 );
  oc_servolwm_rad2pulse0.out( alp0 );
  oc_servolwm_rad2pulse1.out( alp1 );
  robo.commit_all();
  return 0;
}

struct Data_setAlp
{
  float alp_0;
  float alp_1;
};

ReturnCode run_v_loop( const RunLoopState &rls, const RunLoopData &rld, void *data )
{
  if( !data ) {
    return rcFatal;
  }
  // auto d = static_cast<Data_setAlp*>(data);

  auto rc = robo.measure_all();
  if( rc.isError() ) {
    return rc;
  }

  float t = (float) std::max( rls.tc - rld.t_pre, 0_u32 ) * 1e-3f;
  float ph_0 = t / T_0;
  float ph_1 = t / T_1;
  float q_0 = a_0 * sinf( ph_0 );
  float q_1 = a_1 * sinf( ph_1 );

  if( !dry_run ) {
    oc_servolwm_rad2pulse0.out( q_0 );
    oc_servolwm_rad2pulse1.out( q_1 );
    robo.commit_all();
  }

  // float v = ( ( rls.stage & RunLoopState::stage_num_mask ) == 1 ) ? d->v : 0;

  // if( rls.stage & RunLoopState::stage_change_flag  ) {
  //   // q0_actu.setV( v );
  //   robo.commit_all();
  // }

  std_out
    << FmtInt( rls.tc, 8 ) << ' '
    << t << ' ' << ph_0 << ' ' << ph_1 << ' ' << q_0 << ' ' << q_1
    // << FltFmt( robo.get_t_cur(), cvtff_auto, 12, 3 )
    << NL;

  return rcOk;
}



CMD_FUNCTION( run0 ) // R
{
  struct Data_setAlp d;
  d.alp_0      = arg2float_d( 1, argc, argv, 0 );
  d.alp_1      = arg2float_d( 2, argc, argv, 0 );

  RunLoopData rld( t_step, t_pre, t_run_c , t_post );

  robo.start_time();
  auto rc = run_periodic( rld, run_v_loop, &d );

  robo.commit_all();

  return rc.isOk() ? 0 : 2;
}

CMD_FUNCTION( prt )
{
  std_out << "# SERVOLWM: " NL;
  tim_print_cfg( TIM_SERVOLWM_BASE );
  return 0;
}


// ------------------------------------  ------------------------------------------------


