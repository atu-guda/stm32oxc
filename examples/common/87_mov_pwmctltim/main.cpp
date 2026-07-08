#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>
#include <oxc_pingpio.h>
#include <oxc_robopin.h>
#include <oxc_pwmctltim.h>
#include <oxc_robopwmctl.h>
#include <oxc_actu_dcpwm.h>

#include <main.h>

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


void idle_main_task()
{
  leds.toggle( 1_mask );
}

size_t err_idx { 0 };
ReturnCode init_all();
ReturnCode commit_all();
ReturnCode measure_all();

TIM_HandleTypeDef tim_pwm_h;

constinit PwmCtlTim pwm1( TIM_PWM_BASE, tim_pwm_chspins, tim_pwm_h );
RoboPwmCtl q0_pwm( "q0_pwm", pwm1 );

PinGpio pwm_left_pin{  PwmLeftPin  };
PinGpio pwm_right_pin{ PwmRightPin };
RoboPin q0_pin_l{ "q0_pin_l", pwm_left_pin };
RoboPin q0_pin_r{ "q0_pin_r", pwm_right_pin };
LinearCoordTransform q0_coord_tr { 1.986f, 0 }; // TODO: coeff (mech dependent) to header
ActuDcPwm_1P2D q0_actu( q0_pwm, 0, q0_pin_l, q0_pin_r, q0_coord_tr );

RoboDevice* hw_robo_actu[] {
  &q0_pin_l,
  &q0_pin_r,
  &q0_pwm,
};

RoboDevice* hw_robo_sens[] {
};

int main(void)
{
  BOARD_PROLOG;

  UVAR_t =  20;
  UVAR_n = 100;

  init_all();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_all()
{
  // q0:
  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq( TIM_PWM ), 20000 );
  pwm1.setAllowPSCadj( true );
  tim_pwm_h.Instance = TIM_PWM;
  pwm1.setHardParams( psc_i, arr_i );
  pwm1.enable();

  size_t idx { 0 };
  ReturnCode rc { rcOk };
  for( auto dev : hw_robo_actu ) {
    rc = dev->initHW();
    if( rc.isError() ) {
      err_idx = idx;
      return rc;
    }
    ++idx;
  }
  // for( auto dev : hw_robo_sens ) {
  //   dev->initHW();
  //   if( rc.isError() ) {
  //     err_idx = idx;
  //     return rc;
  //   }
  //   ++idx;
  // }
  return rcOk;
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
  float v = arg2float_d( 1, argc, argv, 0 );
  auto  t = arg2ulong_d( 2, argc, argv, 1000, 0 );

  auto rc = q0_actu.setV( v );
  std_out << "# v= " << v << " rc.code= " << rc.code << NL;
  commit_all();
  delay_ms_brk( t );
  std_out << "# v_phy= " << q0_actu.get_v_phy() << " v_int= " << q0_actu.get_v_int()
          << " raw0: " << pwm1.getPwmRaw( 0 ) << NL;

  q0_actu.idle();
  commit_all();

  return 0;
}

CMD_FUNCTION( commit ) // C
{
  return commit_all() ? 0 : 2;
}


ReturnCode measure_all()
{
  for( size_t idx=0; auto dev : hw_robo_sens ) {
    auto rc = dev->measure();
    if( rc.isError() ) { // TODO: param: break on error
      leds[0].set();
      err_idx = idx;
      return rc;
    }
    ++idx;
  }

  // first_measure = 0;
  return rcOk;
}

ReturnCode commit_all()
{
  for( size_t idx=0; auto dev : hw_robo_actu ) {
    auto rc = dev->commit();
    if( !rc.isOk() ) {
      err_idx = idx;
      UVAR_e = idx;
      return rc;
    }
    ++idx;
  }
  return rcOk;
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

