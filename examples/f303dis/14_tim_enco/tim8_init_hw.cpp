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

void HAL_TIM_Encoder_MspInit( TIM_HandleTypeDef* timh_enco )
{
  if( timh_enco->Instance != TIM8 ) {
    return;
  }
  __TIM8_CLK_ENABLE();
  __GPIOC_CLK_ENABLE();

  GPIO_InitTypeDef gpi;

  //*  C6 --> TIM8_CH1, C7 --> TIM8_CH2
  gpi.Pin = GPIO_PIN_6 | GPIO_PIN_7;
  gpi.Mode = GPIO_MODE_AF_PP;
  // gpi.Pull = GPIO_PULLDOWN;
  gpi.Pull = GPIO_NOPULL; // use external resistors
  gpi.Speed = GPIO_SPEED_HIGH;
  gpi.Alternate = GPIO_AF4_TIM8;
  HAL_GPIO_Init( GPIOC, &gpi );
}

void HAL_TIM_PWM_MspDeInit( TIM_HandleTypeDef* timh_enco )
{
  if( timh_enco->Instance != TIM8 ) {
    return;
  }
  __TIM8_CLK_DISABLE();
  HAL_GPIO_DeInit( GPIOC, GPIO_PIN_6 | GPIO_PIN_7 );

  /* Peripheral interrupt Deinit*/
  // HAL_NVIC_DisableIRQ(TIM8_BRK_TIM9_IRQn);
}



