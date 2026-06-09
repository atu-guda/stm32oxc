#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>
#include <oxc_pwmctltim.h>

#include <oxc_main.h>

#include <oxc_actu_servo_c_lwm.h>

#include <main.h>

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


void idle_main_task()
{
  leds.toggle( 1_mask );
}


// constexpr test
// constexpr auto gpioa_idx = GpioA.getIdx();

TIM_HandleTypeDef tim_pwm_h;

constinit PwmCtlTim pwm1( TIM_PWM_BASE, tim_pwm_chspins );

ActuServoContLWM mot0( pwm1, 0 );

int main(void)
{
  BOARD_PROLOG;

  UVAR_t = 100;
  UVAR_n =  20;

  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq( TIM_PWM ), 50 );
  pwm1.setAllowPSCadj( true );
  tim_pwm_h.Instance = TIM_PWM;
  pwm1.initHW( tim_pwm_h, psc_i, arr_i ); // do not really good
  pwm1.initPins();
  pwm1.enable();

  BOARD_POST_INIT_BLINK;

  oxc_add_aux_tick_fun( led_task_nortos );

  std_main_loop_nortos( &srl, idle_main_task );

  return 0;
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

  mot0.setV( v );
  delay_ms_brk( t );
  // std_out << '#' << pu << ' ' << pwm1.getPwmRaw( 0 ) << NL;
  mot0.stop();

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

