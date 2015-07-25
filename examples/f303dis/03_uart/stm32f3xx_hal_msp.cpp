/**
  * File Name          : stm32f3xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  */

#include <stm32f3xx_hal.h>
// #include <oxc_gpio.h> // only for debug (leds)


void HAL_UART_MspInit( UART_HandleTypeDef* huart )
{

  GPIO_InitTypeDef gio;
  if( huart->Instance == USART2 ) {
    // leds.set( BIT6 );
    __USART2_CLK_ENABLE();
    // __GPIOA_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();

    /** USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX // ALT:
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    //gio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    gio.Pin = GPIO_PIN_5 | GPIO_PIN_6;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_HIGH;
    gio.Alternate = GPIO_AF7_USART2;
    // HAL_GPIO_Init( GPIOA, &gio );
    HAL_GPIO_Init( GPIOD, &gio );

    /* Peripheral interrupt init*/
    // wait: not now
    // HAL_NVIC_SetPriority( USART2_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
    // HAL_NVIC_EnableIRQ( USART2_IRQn );
  }

}

void HAL_UART_MspDeInit( UART_HandleTypeDef* huart )
{
  if( huart->Instance==USART2 )  {
    __USART2_CLK_DISABLE();
    // HAL_GPIO_DeInit( GPIOA, GPIO_PIN_2 | GPIO_PIN_3 );
    HAL_GPIO_DeInit( GPIOD, GPIO_PIN_5 | GPIO_PIN_6 );
    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ( USART2_IRQn );
  }

}

