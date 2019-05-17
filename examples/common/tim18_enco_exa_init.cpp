#include <oxc_base.h>

// initialize timer 1 or 8 (from board defines) to
// 2-channel encoder mode

void HAL_TIM_Encoder_MspInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKEN;

  GPIO_InitTypeDef gio;

  gio.Pin       = TIM_EXA_PIN1 | TIM_EXA_PIN2; // see inc/bsp/board_xxxx.h
  gio.Mode      = GPIO_MODE_AF_PP;
  gio.Pull      = GPIO_NOPULL; // use external resistors
  gio.Speed     = GPIO_SPEED_MAX;
  #if  ! defined (STM32F1)
  gio.Alternate = TIM_EXA_GPIOAF;
  #endif
  HAL_GPIO_Init( TIM_EXA_GPIO, &gio );
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* htim )
{
  if( htim->Instance != TIM_EXA ) {
    return;
  }
  TIM_EXA_CLKDIS;
  HAL_GPIO_DeInit( TIM_EXA_GPIO, TIM_EXA_PIN1 | TIM_EXA_PIN2 );
  // HAL_NVIC_DisableIRQ( TIM_EXA_IRQ );
}

