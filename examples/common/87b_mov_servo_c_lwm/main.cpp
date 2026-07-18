#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>
#include <oxc_pwmctltim.h>

#include <oxc_main.h>

#include <oxc_actu_servo_c_lwm.h>
#include <oxc_sensor_encoder.h>

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
DCL_CMD_REG(     setRef,  'Z',     " [v] - set [zero] encoder value"  );


size_t err_idx { 0 };
// TODO: to main object
ReturnCode init_all();
ReturnCode commit_all();
ReturnCode measure_all();

void idle_main_task()
{
  leds.toggle( 1_mask );
}

TIM_HandleTypeDef tim_encoder_h;
EncoderProxyAddr q0_enc_proxy( TIM_ENCODER_BASE + tim_cnt_offset, false );
SensorEncoder q0_sens_hw( "q0_enco", q0_enc_proxy, 2048, true, 0xFFFF ); // true - reverse
LinearCoordTransform q0_sens_tr { 2 * pi_f / q0_sens_hw.getScale(0), 0 };
SensorBase q0_sens( q0_sens_hw, 0, q0_sens_tr );


TIM_HandleTypeDef tim_servolwm_h;
constinit PwmCtlTim pwm1( TIM_SERVOLWM_BASE, tim_SERVOLWM_chspins, tim_servolwm_h );
RoboPwmCtl q0_pwm( "q0_pwm", pwm1 );
LinearCoordTransform q0_coord_tr { 1.986f, 0 }; // TODO: coeff (mech dependent) to header
ActuServoContLWM q0_actu( q0_pwm, 0, q0_coord_tr );

RoboDevice* hw_robo_actu[] {
  &q0_pwm,
};

RoboDevice* hw_robo_sens[] {
  &q0_sens_hw,
};


int main(void)
{
  BOARD_PROLOG;

  UVAR_t =  20;
  UVAR_n =  50;

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
  tim_servolwm_h.Instance = TIM_SERVOLWM;
  pwm1.setHardParams( psc_i, arr_i, TIM_COUNTERMODE_UP );
  pwm1.enable();

  tim_encoder_h.Instance = TIM_ENCO;
  tim_enco_cfg_default( tim_encoder_h );

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
  std_out << "# LWM freq:  "  << pwm1.getFreq() << NL;

  tim_print_cfg( TIM_SERVOLWM_BASE );

  dump32( (void*)TIM_SERVOLWM_BASE, 0x60 );

  std_out << "# Encoder:  " << TIM_ENCO->CNT
          <<  " hw:" << q0_sens_hw.get(0) << " log: " << q0_sens.get() <<NL;
  tim_print_cfg( TIM_ENCODER_BASE );

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
  auto  n = arg2ulong_d( 2, argc, argv, UVAR_n, 0 );
  uint32_t t_step = UVAR_t;


  q0_actu.setV( v );
  commit_all();

  break_flag = 0;
  uint32_t tm0 { GET_OS_TICK() }, tm00 { tm0 };
  for( uint32_t i=0; i <= n && !break_flag; ++i ) {
    uint32_t tcc = GET_OS_TICK() - tm00;
    auto rc = measure_all();
    if( rc.isError() ) {
      break_flag = 2; break;
    }
    std_out << FmtInt( tcc, 8 ) << ' '  << v << ' ' << pwm1.getPwmRaw( 0 )
            << ' ' << q0_actu.get_v_int() << ' ' << q0_actu.get_v_phy() << ' '
            << q0_sens_hw.get(0) << ' ' << r2d( q0_sens.get() ) << NL;

    delay_ms_until_brk( &tm0, t_step );
  }
  q0_actu.idle();
  commit_all();

  return 0;
}

CMD_FUNCTION( setRef ) // Z
{
  auto nv = arg2long_d( 1, argc, argv, 0 );
  auto v0 = q0_sens_hw.get( 0 );
  q0_sens_hw.setVal( 0, nv );
  auto v1 = q0_sens_hw.get( 0 );
  std_out << "# old: " << v0 << " new: " << v1 << NL;
  return 0;
}


// ----------------------- encoder timer part ---------------


// -----------------------  timers init part ---------------

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_SERVOLWM ) {
    TIM_SERVOLWM_CLKEN();
    return;
  }
  if( htim->Instance == TIM_ENCO ) {
    TIM_ENCODER_CLKEN();
    return;
  }
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance == TIM_SERVOLWM ) {
    TIM_SERVOLWM_CLKDIS();
    return;
  }
  if( htim->Instance == TIM_ENCO ) {
    TIM_ENCODER_CLKDIS();
    return;
  }
}

void HAL_TIM_Encoder_MspInit( TIM_HandleTypeDef* tim_encoderHandle )
{
  if( tim_encoderHandle->Instance == TIM_ENCO ) {
    TIM_ENCODER_CLKEN();
    ENCODER_EncoPin_A.cfgAF( ENCODER_AF );
    ENCODER_EncoPin_B.cfgAF( ENCODER_AF );
  }
}


