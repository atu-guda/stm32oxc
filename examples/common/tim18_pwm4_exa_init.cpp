#include <oxc_base.h>
#include <oxc_gpio.h>

// initialize exmple timer (from board defines) to
// 4-channel PWM
// not always TIM / TIM8 TODO: rename file + fix Makefiles

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  TIM_EXA_GPIO.cfgAF_N( TIM_EXA_PINS, TIM_EXA_GPIOAF );

  // if one timer uses different AF/GPIO, like F334:T1
  #ifdef TIM_EXA_PINS_EXT
    TIM_EXA_GPIO_EXT.cfgAF_N( TIM_EXA_PINS_EXT, TIM_EXA_GPIOAF );
  #endif
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  TIM_EXA_GPIO.cfgIn_N( TIM_EXA_PINS );
  // HAL_NVIC_DisableIRQ( TIM_EXA_IRQ );
}

