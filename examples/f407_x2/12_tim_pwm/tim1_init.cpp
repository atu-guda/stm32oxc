#include <oxc_base.h>

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  GPIO_InitTypeDef gio;

  gio.Pin = TIM_EXA_PINS; // see inc/bsp/board_xxxx.h
  gio.Mode = GPIO_MODE_AF_PP;
  gio.Pull = GPIO_NOPULL;
  gio.Speed = GPIO_SPEED_HIGH;
  gio.Alternate = TIM_EXA_GPIOAF;
  HAL_GPIO_Init( TIM_EXA_GPIO, &gio );

  // HAL_NVIC_SetPriority( TIM_EXA_IRQ, 14, 0 );
  // HAL_NVIC_EnableIRQ( TIM_EXA_IRQ );
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  HAL_GPIO_DeInit( TIM_EXA_GPIO, TIM_EXA_PINS );

  // HAL_NVIC_DisableIRQ( TIM_EXA_IRQ );
}

