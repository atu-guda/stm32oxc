#include <climits>
#include <oxc_auto.h>
#include <oxc_cpptypes.h>
#include <oxc_atleave.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>
#include <oxc_pwmctltim.h>
#include <oxc_motorpwm.h>
#include <oxc_as5600.h>

#include <main.h>

using namespace oxc;
using namespace SMLRL;

USE_DIE4LED_ERROR_HANDLER;
BOARD_DEFINE_LEDS;

BOARD_CONSOLE_DEFINES;

const char* common_help_string = "Appication to test PWM motor + AS5600 encoder" NL;

uint32_t last_cmd_end_tick     {     0 };
uint32_t off_motor_idle_ticks  { 60000 };
uint32_t last_measure_tick     {     0 };
uint32_t measure_idle_ticks    {   100 };

int t_step                     {   50  };

// --- local commands;
DCL_CMD_REG(      test0,  'T',     " [arg ] - test something"  );
DCL_CMD_REG(      tinfo,  'P',     " print info"  );
DCL_CMD_REG(    setfreq,  'F',     " Hz - set freq"  );
DCL_CMD_REG(      pulse,  'U',     " []- test pulse in us"  );
DCL_CMD_REG(       setV,  'V',     " v [t_us] - set v"  );
DCL_CMD_REG(    measure,  'M',     " - measure angle..."  );

ReturnCode measure_store_coords();


I2C_HandleTypeDef i2ch;
DevI2C i2cd( &i2ch, 0 );
AS5600 ang_sens( i2cd );


void init_mot0();

TIM_HandleTypeDef tim_pwm_h;

constinit PwmCtlTim pwm1( TIM_PWM_BASE, tim_pwm_chspins );

PinGpio pwm_left_pin(  PwmLeftPin  );
PinGpio pwm_right_pin( PwmRightPin );
MotorPwm1P2D mot0( pwm1, 0, pwm_left_pin, pwm_right_pin );

void idle_main_task()
{
  const uint32_t t = HAL_GetTick();
  if( t - last_cmd_end_tick > off_motor_idle_ticks ) {
    // stop();
    last_cmd_end_tick = t;
  }
  if( ( t - last_measure_tick ) > measure_idle_ticks ) {
    measure_store_coords();
  }
}

int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 100;
  UVAR_n =  20;

  UVAR_e = i2c_default_init( i2ch /*, 400000 */ );
  i2c_dbg = &i2cd;
  i2c_client_def = &ang_sens;
  ang_sens.setCfg( AS5600::CfgBits::cfg_pwr_mode_nom |  AS5600::CfgBits::cfg_hyst_off );

  if( !ang_sens.isMagnetDetected()  ) {
    std_out << "# Error: no magnet" << NL;
    die4led( 1_mask );
  }

  init_mot0();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
}

void init_mot0()
{
  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq( TIM_PWM ), 20000 );
  pwm1.setAllowPSCadj( true );
  tim_pwm_h.Instance = TIM_PWM;
  pwm1.initHW( tim_pwm_h, psc_i, arr_i );
  mot0.initHW();
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
  float v = arg2float_d( 1, argc, argv, 0 );
  auto  t = arg2ulong_d( 2, argc, argv, 1000, 0 );

  mot0.set_v( v );
  delay_ms_brk( t );
  // std_out << '#' << pu << ' ' << pwm1.getPwmRaw( 0 ) << NL;
  mot0.stop();

  return 0;
}


CMD_FUNCTION( measure ) // M
{
  std_out << ang_sens.getAngleN() << ' ' << ang_sens.isMagnetDetected() << NL;

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

ReturnCode measure_store_coords()
{
  auto old_tick { last_measure_tick };
  last_measure_tick = HAL_GetTick();
  // auto dt = std::max( last_measure_tick - old_tick, 2_u32 );

  leds[2].set();
  DoAtLeave _( []() { leds[2].reset(); } );

  ang_sens.getAngleN();

  // for( auto ps : sensors ) {
  //   auto rc = ps->measure( adc_n );
  //   if( rc < 1 ) {
  //     ledsx[0].set();
  //     return 0;
  //   }
  // }
  //
  // for( auto &co : coords ) {
  //   auto q_o = co.q_cur;
  //   co.q_cur = co.sens->get( co.sens_ch );
  //   co.nu_cur = 1000 * ( co.q_cur - q_o ) / dt; // 1000 is ms/s
  // }

  return rcOk;
}

