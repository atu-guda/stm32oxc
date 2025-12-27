#include <oxc_base.h>
#include <oxc_gpio.h>

// initialize timer 1 or 8 (from board defines) to
// 2-channel ultrasonic range sensor

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  TIM_EXA_GPIO.cfgAF_N( TIM_EXA_PIN1 | TIM_EXA_PIN2, TIM_EXA_GPIOAF );

}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  TIM_EXA_GPIO.cfgIn_N( TIM_EXA_PIN1 | TIM_EXA_PIN2 );
  // HAL_NVIC_DisableIRQ( TIM_EXA_IRQ );
}

