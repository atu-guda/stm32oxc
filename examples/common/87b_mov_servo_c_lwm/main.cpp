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

  UVAR_l =    1; // idLe after run
  UVAR_t =   20; // t_step, ms
  UVAR_n =   50;
  UVAR_p =  200; // t_pre,  ms
  UVAR_r = 1000; // t_run,  ms, def
  UVAR_o =  500; // t_post, ms

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

// XXXXXXXXXX

struct RunLoopData
{
  uint32_t t_step;
  uint32_t t_pre;
  uint32_t t_run;
  uint32_t t_post;
  uint32_t t_12;  // stage  1(run)  -> 2(post)
  uint32_t t_end; // stage  2(post) -> 3(end)
  void init_n_step( uint32_t n, uint32_t t_step_ ) {
    t_step = t_step_; t_pre = 0; t_run = n * t_step; t_post = 0; t_12 = t_end = t_run;
  }
  void init_t( uint32_t t_step_, uint32_t t_pre_, uint32_t t_run_, uint32_t t_post_ ) {
    t_step = t_step_; t_pre = t_pre_; t_run = t_run_; t_post = t_post_;
    t_12 = t_pre + t_run; t_end = t_12 + t_post;
  }
};

struct RunLoopState
{
  enum  { stage_change_flag   = 0x8000, stage_num_mask = 0x0FFF, stage_pre = 0, stage_run = 1, stage_post = 2, stage_end = 3 };
  uint32_t i     ; //* iteration
  uint32_t t     ; //* near i * t_step, ms
  uint32_t tc    ; //* measured time, ms
  uint32_t stage ; //* pre + change 0:pre, 1: run, 2: post
};

using run_periodic_fun = ReturnCode (*)( const RunLoopState &rls, const RunLoopData &rld, void *data );

// atu:
ReturnCode run_periodic( const RunLoopData &rld, run_periodic_fun fun, void *data )
{
  RunLoopState rls;

  uint32_t tm0 { GET_OS_TICK() };
  const uint32_t tm00 { tm0 };

  break_flag = 0;
  rls.i = 0; rls.stage = RunLoopState::stage_change_flag;
  for( rls.t = 0; rls.t <= rld.t_end && !break_flag; rls.t += rld.t_step, ++rls.i ) {
    rls.tc = GET_OS_TICK() - tm00;
    if( ( rls.stage & RunLoopState::stage_num_mask ) == 0 && rls.t >= rld.t_pre ) { // switch to run
      rls.stage = 1 | RunLoopState::stage_change_flag;
    }
    if( ( rls.stage & RunLoopState::stage_num_mask ) == 1 && rls.t >= rld.t_12 ) { // switch to post
      rls.stage = 2 | RunLoopState::stage_change_flag;
    }

    auto rc = fun( rls, rld, data );
    if( !rc.isOk () ) {
      break_flag = 2; break;
    }
    rls.stage &= RunLoopState::stage_num_mask;
    delay_ms_until_brk( &tm0, rld.t_step );
  }
  // rls.stage = 3 | RunLoopState::stage_change_flag; // unused, as rls dropped
  return break_flag ? rcErr : rcOk;
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

  auto rc = measure_all();
  if( rc.isError() ) {
    return rc;
  }
  float v = ( ( rls.stage & RunLoopState::stage_num_mask ) == 1 ) ? d->v : 0;

  if( rls.stage & RunLoopState::stage_change_flag  ) {
    q0_actu.setV( v );
    commit_all();
  }

  std_out << FmtInt( rls.tc, 8 ) << ' '  << v << ' ' << pwm1.getPwmRaw( 0 )
    << ' ' << q0_actu.get_v_int() << ' ' << q0_actu.get_v_phy() << ' '
    << q0_sens_hw.get(0) << ' ' << r2d( q0_sens.get() )
    << ' ' << HexInt(rls.stage) << NL;

  return rcOk;
}

CMD_FUNCTION( setV ) // V
{
  struct Data_setV d;
  d.v        = arg2float_d( 1, argc, argv, 0 );
  auto t_run = arg2ulong_d( 2, argc, argv, UVAR_r, 0 );

  RunLoopData rld;
  rld.init_t( UVAR_t, UVAR_p, t_run, UVAR_o );
  // std_out << "# rld: " << rld.t_step << ' ' << rld.t_pre << ' ' << rld.t_run 
  //  << ' ' << rld.t_post << ' ' << rld.t_12 << ' ' << rld.t_end << NL;

  auto rc = run_periodic( rld, run_v_loop, &d );

  if( UVAR_l ) {
    q0_actu.idle();
    commit_all();
  }

  return rc.isOk() ? 0 : 2;
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


