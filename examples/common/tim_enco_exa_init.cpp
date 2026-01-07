#include <oxc_base.h>
#include <oxc_gpio.h>

// initialize timer from board defines to
// 2-channel encoder mode

void HAL_TIM_Encoder_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_IN_EXA ) {
    return;
  }
  TIM_IN_EXA_CLKEN;

  TIM_IN_EXA_PIN1.cfgAF( TIM_IN_EXA_GPIOAF );
  TIM_IN_EXA_PIN2.cfgAF( TIM_IN_EXA_GPIOAF );
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_IN_EXA ) {
    return;
  }
  TIM_IN_EXA_CLKDIS;
  TIM_IN_EXA_PIN1.cfgIn();
  TIM_IN_EXA_PIN2.cfgIn();
  // HAL_NVIC_DisableIRQ( TIM_IN_EXA_IRQ );
}

