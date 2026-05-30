#include <climits>
#include <oxc_auto.h>
#include <oxc_floatfun.h>
#include <oxc_main.h>
#include <oxc_pwmctltim.h>

#include <oxc_main.h>

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


void idle_main_task()
{
  leds.toggle( 1_mask );
}


// constexpr test
// constexpr auto gpioa_idx = GpioA.getIdx();

TIM_HandleTypeDef tim_pwm_h;
ReturnCode tim_pwm_cfg_common( TIM_HandleTypeDef &t_h, uint32_t psc, uint32_t arr, std::span<const TimChPin> chpins );

PinGpio pwm_left_pin(  PwmLeftPin  );
PinGpio pwm_right_pin( PwmRightPin );
// PinOut pwm_pwm_pin( PwmPwmPin );

constinit PwmCtlTim pwm1( TIM_PWM_BASE, tim_pwm_chspins );

int main(void)
{
  BOARD_PROLOG;


  UVAR_t = 100;
  UVAR_n =  20;
  // UVAR_a = arr_i;
  // UVAR_p = psc_i;

  pwm_left_pin.initHW();
  pwm_right_pin.initHW();

  tim_pwm_h.Instance = TIM_PWM;
  auto [ psc_i, arr_i ] = calc_tim_psc_arr( get_TIM_in_freq( TIM_PWM ), 20000 );
  UVAR_z = tim_pwm_cfg_common( tim_pwm_h, psc_i, arr_i, tim_pwm_chspins );
  // tim_cfg( UVAR_p, UVAR_a );
  pwm1.setAllowPSCadj( true );
  // pwm1.initHW( psc_i, arr_i ); // do not really good
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

// TODO: move to lib
static const constexpr TIM_OC_InitTypeDef tim_oc_cfg_default {
  .OCMode       = TIM_OCMODE_PWM1,
  .Pulse        = 0,
  .OCPolarity   = TIM_OCPOLARITY_HIGH,
  .OCNPolarity  = TIM_OCNPOLARITY_HIGH,
  .OCFastMode   = TIM_OCFAST_DISABLE,
  .OCIdleState  = TIM_OCIDLESTATE_RESET,
  .OCNIdleState = TIM_OCNIDLESTATE_RESET,
};
static const constexpr TIM_ClockConfigTypeDef sClockSourceConfig_def {
  .ClockSource    = TIM_CLOCKSOURCE_INTERNAL,
  .ClockPolarity  = 0, // ignored
  .ClockPrescaler = 0,
  .ClockFilter    = 0
};
static const constexpr TIM_MasterConfigTypeDef sMasterConfig_def {
  .MasterOutputTrigger  = TIM_TRGO_UPDATE,
  #ifdef TIM_TRGO2_ENABLE
  .MasterOutputTrigger2 = TIM_TRGO2_UPDATE,
  #endif
  .MasterSlaveMode      = TIM_MASTERSLAVEMODE_DISABLE,
};


//  t_h.Instance must be set beforehand;
ReturnCode tim_pwm_cfg_common( TIM_HandleTypeDef &t_h, uint32_t psc, uint32_t arr, std::span<const TimChPin> chpins )
{
  if( t_h.Instance == nullptr ) {
    return rcFatal;
  }

  t_h.Init.Prescaler         = psc;
  t_h.Init.Period            = arr;
  t_h.Init.ClockDivision     = 0;
  t_h.Init.CounterMode       = TIM_COUNTERMODE_UP;
  t_h.Init.RepetitionCounter = 0;
  if( HAL_TIM_PWM_Init( &t_h ) != HAL_OK ) {
    UVAR_e = 1; // like error
    return rcErr;
  }

  HAL_TIM_ConfigClockSource( &t_h, &sClockSourceConfig_def );

  if( HAL_TIMEx_MasterConfigSynchronization( &t_h, &sMasterConfig_def ) != HAL_OK ) {
    UVAR_e = 2;
    return rcErr;
  }

  for( auto ch : chpins ) {
    if( HAL_TIM_PWM_ConfigChannel( &t_h, &tim_oc_cfg_default, TimCh::ch2hal_ch(ch.ch) ) != HAL_OK ) {
      UVAR_e = 3000;
      return rcErr;
    }
    HAL_TIM_PWM_Start( &t_h, TimCh::ch2hal_ch(ch.ch) );
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
