#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>
#include <oxc_pingpio.h>
#include <oxc_robopin.h>
#include <oxc_pwmctltim.h>
#include <oxc_robopwmctl.h>
#include <oxc_actu_dcpwm.h>

#include <board_robo_cfg.h>

using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test misc movers" NL;

// --- local commands;
DCL_CMD_REG(      test0,  'T',     " [arg ] - test something"  );
DCL_CMD_REG(      tinfo,  'P',     " print info"  );
DCL_CMD_REG(    setfreq,  'F',     " Hz - set freq"  );
DCL_CMD_REG(      pulse,  'U',     " []- test pulse in us"  );
DCL_CMD_REG(       setV,  'V',     " v [t_us] - set v as robo"  );
DCL_CMD_REG(     commit,  'C',     " commit all hw devices"  );


ReturnCode init_hw_all();

void idle_main_task()
{
  leds.toggle( 1_mask );
}


TIM_HandleTypeDef tim_pwm_h;

constinit PwmCtlTim pwm1( TIM_MPWM_BASE, tim_MPWM_chspins, tim_pwm_h );
RoboPwmCtl pwm1_ctl( "pwm1_ctl", pwm1 );

PinGpio pwm_left_pin{  MPWM_CtlPin_L  };
PinGpio pwm_right_pin{ MPWM_CtlPin_R };
RoboPin q0_pin_l{ "q0_pin_l", pwm_left_pin };
RoboPin q0_pin_r{ "q0_pin_r", pwm_right_pin };
LinearCoordTransform q0_coord_tr { 1.986f, 0 }; // TODO: coeff (mech dependent) to header
ActuDcPwm_1P2D q0_actu( pwm1_ctl, 0, q0_pin_l, q0_pin_r, q0_coord_tr );

RoboDevice* hw_robo_devs[] {
  &q0_pin_l,
  &q0_pin_r,
  &pwm1_ctl,
};

RoboJoint fake_joint;

RoboJoint* robo_joints[] {
  &fake_joint,
};

RoboAssembly robo( hw_robo_devs, robo_joints );


int main(void)
{
  BOARD_PROLOG;

  UVAR_l =    1; // idLe after run
  UVAR_t =   20; // t_step, ms
  UVAR_n =   50;
  UVAR_p =  200; // t_pre,  ms
  UVAR_r = 1000; // t_run,  ms, def
  UVAR_o =  500; // t_post, ms

  init_hw_all();


  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_hw_all()
{
  // q0:
  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq( TIM_MPWM_BASE ), 20000 );
  pwm1.setAllowPSCadj( true );
  tim_pwm_h.Instance = TIM_MPWM;
  pwm1.setHardParams( psc_i, arr_i, TIM_COUNTERMODE_UP );
  pwm1.enable();

  return robo.init_all();
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

  auto rc = robo.measure_all();
  if( rc.isError() ) {
    return rc;
  }
  float v = ( ( rls.stage & RunLoopState::stage_num_mask ) == 1 ) ? d->v : 0;

  if( rls.stage & RunLoopState::stage_change_flag  ) {
    q0_actu.setV( v );
    robo.commit_all();
  }

  std_out << FmtInt( rls.tc, 8 ) << ' '  << v << ' ' << pwm1.getPwmRaw( 0 )
    << ' ' << q0_actu.get_v_int() << ' ' << q0_actu.get_v_phy() << ' '
    // << q0_sens_hw.get(0) << ' ' << r2d( q0_sens.get() )
    << NL;

  return rcOk;
}


CMD_FUNCTION( setV ) // V
{
  struct Data_setV d;
  d.v        = arg2float_d( 1, argc, argv, 0 );
  auto t_run = arg2ulong_d( 2, argc, argv, UVAR_r, 0 );

  RunLoopData rld( UVAR_t, UVAR_p, t_run, UVAR_o );

  auto rc = run_periodic( rld, run_v_loop, &d );

  if( UVAR_l ) {
    q0_actu.idle();
    robo.commit_all();
  }

  return rc.isOk() ? 0 : 2;
}

CMD_FUNCTION( commit ) // C
{
  return robo.commit_all().isOk() ? 0: 2;
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

