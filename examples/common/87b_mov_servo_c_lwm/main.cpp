#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>
#include <oxc_pwmctltim.h>

#include <oxc_main.h>

#include <oxc_actu_servo_c_lwm.h>

#include <board_robo_cfg.h>

using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test contigous LWM servo" NL;

// --- local commands;
DCL_CMD_REG(      test0,  'T',     " [arg ] - test something"  );
DCL_CMD_REG(      tinfo,  'P',     " print info"  );
DCL_CMD_REG(    setfreq,  'F',     " Hz - set freq"  );
DCL_CMD_REG(      pulse,  'U',     " []- test pulse in us"  );
DCL_CMD_REG(       setV,  'V',     " v [t_us] - set v"  );


size_t err_idx { 0 };
// TODO: to main object
ReturnCode init_all();
ReturnCode commit_all();
ReturnCode measure_all();

void idle_main_task()
{
  leds.toggle( 1_mask );
}



TIM_HandleTypeDef tim_servolwm_h;
constinit PwmCtlTim pwm1( TIM_SERVOLWM_BASE, tim_SERVOLWM_chspins, tim_servolwm_h );
RoboPwmCtl q0_pwm( "q0_pwm", pwm1 );
LinearCoordTransform q0_coord_tr { 1.986f, 0 }; // TODO: coeff (mech dependent) to header
ActuServoContLWM q0_actu( q0_pwm, 0, q0_coord_tr );

RoboDevice* hw_robo_actu[] {
  &q0_pwm,
};

RoboDevice* hw_robo_sens[] {
};


int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 100;
  UVAR_n =  20;

  init_all();


  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

ReturnCode init_all()
{
  // q0:
  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq( TIM_SERVOLWM_BASE ), 50 );
  pwm1.setAllowPSCadj( true );
  tim_servolwm_h.Instance = addr2TIM( TIM_SERVOLWM_BASE );
  pwm1.setHardParams( psc_i, arr_i, TIM_COUNTERMODE_UP );
  pwm1.enable();
  pwm1.initPins(); // ??

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
  for( auto dev : hw_robo_sens ) {
    dev->initHW();
    if( rc.isError() ) {
      err_idx = idx;
      return rc;
    }
    ++idx;
  }
  return rcOk;
}

ReturnCode measure_all()
{
  size_t idx { 0 };
  for( auto dev : hw_robo_sens ) {
    auto rc = dev->measure();
    if( rc.isError() ) { // TODO: param: break on error
      leds[0].set();
      err_idx = idx;
      return rc;
    }
  }

  return rcOk;
}

ReturnCode commit_all()
{
  size_t idx { 0 };
  for( auto dev : hw_robo_actu ) {
    auto rc = dev->commit();
    if( rc.isError() ) { // TODO: param: break on error
      err_idx = idx;
      return rc;
    }
  }
  return rcOk;
}




CMD_FUNCTION( test0 )
{
  float pwm_v = arg2float_d( 1, argc, argv, 0.5f, 0.0f, 1.0f );
  int v0 = arg2long_d( 2, argc, argv,  UVAR_v, INT_MIN, INT_MAX );

  pwm1.setPwm( 0, pwm_v );

  return v0;
}

CMD_FUNCTION( tinfo ) // P
{
  tim_print_cfg( TIM_SERVOLWM_BASE );

  std_out << "# freq:  "  << pwm1.getFreq() << NL;

  dump32( (void*)TIM_SERVOLWM_BASE, 0x60 );

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

  tim_print_cfg( TIM_SERVOLWM_BASE );

  return 0;
}

CMD_FUNCTION( setV ) // V
{
  float v = arg2float_d( 1, argc, argv, 0 );
  auto  t = arg2ulong_d( 2, argc, argv, 1000, 0 );

  q0_actu.setV( v );
  commit_all();
  delay_ms_brk( t );
  std_out << '#' << v << ' ' << pwm1.getPwmRaw( 0 )
          << ' ' << q0_actu.get_v_int() << ' ' << q0_actu.get_v_phy() << NL;
  q0_actu.idle();
  commit_all();

  return 0;
}



void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == addr2TIM( TIM_SERVOLWM_BASE ) ) {
    TIM_SERVOLWM_CLKEN();
    return;
  }
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == addr2TIM( TIM_SERVOLWM_BASE ) ) {
    TIM_SERVOLWM_CLKDIS();
    return;
  }
}

