/**
  * File Name          : stm32f3xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  */

#include <stm32f3xx_hal.h>

// #include <oxc_gpio.h> // only for debug

#include <FreeRTOS.h>

// --------------------------- TIM --------------------------------------

// void HAL_TIM_Base_MspInit( TIM_HandleTypeDef* htim_base )
// {
// }


// void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
// {
//   if( htim_base->Instance == TIM8 ) {
//     __TIM8_CLK_DISABLE();
//     HAL_GPIO_DeInit( GPIOC, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9 );
//   }
// }

void HAL_TIM_PWM_MspInit( TIM_HandleTypeDef* htim_oc )
{
  // leds.set( LED_BSP_RED_1 );
  GPIO_InitTypeDef gio;
  if( htim_oc->Instance == TIM8 ) {
    __GPIOC_CLK_ENABLE();
    __TIM8_CLK_ENABLE();

    //* TIM8 GPIO Configuration C6:C8 = TIM8_CH1:CH8
    gio.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_HIGH;
    gio.Alternate = GPIO_AF4_TIM8;
    HAL_GPIO_Init( GPIOC, &gio );

    // reserved for future
    // gio.Pin = GPIO_PIN_7;
    // gio.Mode = GPIO_MODE_AF_PP;
    // gio.Pull = GPIO_PULLDOWN;
    // gio.Speed = GPIO_SPEED_HIGH;
    // gio.Alternate = GPIO_AF4_TIM8;
    // HAL_GPIO_Init(GPIOC, &gio);
  }
}


// void HAL_TIM_OC_MspInit( TIM_HandleTypeDef* htim_oc )
// {
// }


