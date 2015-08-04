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


// --------------------------- UART --------------------------------------

void HAL_UART_MspInit( UART_HandleTypeDef* huart )
{

  GPIO_InitTypeDef gio;
  if( huart->Instance == USART2 ) {
    __USART2_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    //__GPIOD_CLK_ENABLE();

    /** USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX // ALT:
    PD5     ------> USART2_TX
    PD6     ------> USART2_RX
    */
    gio.Pin = GPIO_PIN_2 | GPIO_PIN_3;
    //gio.Pin = GPIO_PIN_5 | GPIO_PIN_6;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_HIGH;
    gio.Alternate = GPIO_AF7_USART2;
    HAL_GPIO_Init( GPIOA, &gio );
    // HAL_GPIO_Init( GPIOD, &gio );

    /* Peripheral interrupt init*/
    HAL_NVIC_SetPriority( USART2_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
    HAL_NVIC_EnableIRQ( USART2_IRQn );
  }

}

void HAL_UART_MspDeInit( UART_HandleTypeDef* huart )
{
  if( huart->Instance==USART2 ) {
    __USART2_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_2 | GPIO_PIN_3 );
    // HAL_GPIO_DeInit( GPIOD, GPIO_PIN_5 | GPIO_PIN_6 );
    /* Peripheral interrupt Deinit*/
    HAL_NVIC_DisableIRQ( USART2_IRQn );
  }

}


int init_uart( UART_HandleTypeDef *uahp, int baud )
{
  uahp->Instance = USART2;
  uahp->Init.BaudRate     = baud;
  uahp->Init.WordLength   = UART_WORDLENGTH_8B;
  uahp->Init.StopBits     = UART_STOPBITS_1;
  uahp->Init.Parity       = UART_PARITY_NONE;
  uahp->Init.HwFlowCtl    = UART_HWCONTROL_NONE;
  uahp->Init.Mode         = UART_MODE_TX_RX;
  uahp->Init.OverSampling = UART_OVERSAMPLING_16;
  uahp->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  return( HAL_UART_Init( uahp ) == HAL_OK );
}

