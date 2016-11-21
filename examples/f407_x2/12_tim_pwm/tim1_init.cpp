#include <oxc_base.h>

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim_pwm )
{
  if( htim_pwm->Instance !=TIM1 ) {
    return;
  }
  __TIM1_CLK_ENABLE();
  __GPIOE_CLK_ENABLE();

  GPIO_InitTypeDef gpi;

  /**TIM1 GPIO Configuration
    PE9      ------> TIM1_CH1
    PE11     ------> TIM1_CH2
    PE13     ------> TIM1_CH3
    PE14     ------> TIM1_CH4
    */
  gpi.Pin = GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_13|GPIO_PIN_14;
  gpi.Mode = GPIO_MODE_AF_PP;
  gpi.Pull = GPIO_PULLDOWN;
  gpi.Speed = GPIO_SPEED_HIGH;
  gpi.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init( GPIOE, &gpi );

  // gpi.Pin = GPIO_PIN_13|GPIO_PIN_14;
  // gpi.Mode = GPIO_MODE_AF_PP;
  // gpi.Pull = GPIO_NOPULL;
  // gpi.Speed = GPIO_SPEED_LOW;
  // gpi.Alternate = GPIO_AF1_TIM1;
  // HAL_GPIO_Init(GPIOE, &gpi);

  /* Peripheral interrupt init*/
  /* Sets the priority grouping field */
  // HAL_NVIC_SetPriority( TIM1_BRK_TIM9_IRQn, 14, 0 );
  // HAL_NVIC_EnableIRQ( TIM1_BRK_TIM9_IRQn );

}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim_pwm )
{
  if( htim_pwm->Instance !=TIM1 ) {
    return;
  }
  __TIM1_CLK_DISABLE();
  HAL_GPIO_DeInit( GPIOE, GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_13|GPIO_PIN_14 );

  /* Peripheral interrupt Deinit*/
  // HAL_NVIC_DisableIRQ(TIM1_BRK_TIM9_IRQn);
}

