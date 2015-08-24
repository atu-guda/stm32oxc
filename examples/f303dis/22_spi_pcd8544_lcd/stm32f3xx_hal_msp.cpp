/**
  * File Name          : stm32f3xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  */

#include <stm32f3xx_hal.h>

#include <FreeRTOS.h>

// --------------------------- SPI --------------------------------------

extern SPI_HandleTypeDef spi2_h; // in main.c

int MX_SPI2_Init()
{
  spi2_h.Instance = SPI2;
  spi2_h.Init.Mode = SPI_MODE_MASTER;
  spi2_h.Init.Direction = SPI_DIRECTION_2LINES;
  spi2_h.Init.DataSize = SPI_DATASIZE_8BIT;
  spi2_h.Init.CLKPolarity = SPI_POLARITY_LOW;
  // spi2_h.Init.CLKPolarity = SPI_POLARITY_HIGH;
  spi2_h.Init.CLKPhase = SPI_PHASE_1EDGE;
  // spi2_h.Init.CLKPhase = SPI_PHASE_2EDGE;
  spi2_h.Init.NSS = SPI_NSS_SOFT;
  spi2_h.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  // spi2_h.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  spi2_h.Init.FirstBit = SPI_FIRSTBIT_MSB;
  spi2_h.Init.TIMode = SPI_TIMODE_DISABLED;
  spi2_h.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  return HAL_SPI_Init( &spi2_h );
}

void HAL_SPI_MspInit( SPI_HandleTypeDef *spi_h )
{
  GPIO_InitTypeDef gio;
  if( spi_h->Instance == SPI2 ) {
    __SPI2_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();

    // SPI2 GPIO pins: B12: soft nss(init by PinsOut), B13: SCK, B14: MISO, B15: MOSI
    gio.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_HIGH;
    gio.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init( GPIOB, &gio );
  }
}

void HAL_SPI_MspDeInit( SPI_HandleTypeDef *spi_h )
{
  if( spi_h->Instance == SPI2 ) {
    __SPI2_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 );
  }
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

