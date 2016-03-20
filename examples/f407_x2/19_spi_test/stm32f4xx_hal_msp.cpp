/**
  * File Name          : stm32f4xx_hal_msp.c
  * Description        : This file provides code for the MSP Initialization
  *                      and de-Initialization codes.
  ******************************************************************************
  */

// #include <stm32f4xx_hal.h>
// #include <usbd_core.h>

#include <oxc_base.h>

void default_USBFS_MspInit(void);

void HAL_PCD_MspInit( PCD_HandleTypeDef *hpcd UNUSED_ARG )
{
  default_USBFS_MspInit();
}


// --------------------------- SPI --------------------------------------

extern SPI_HandleTypeDef spi1_h; // in main.c

int MX_SPI1_Init()
{
  spi1_h.Instance = SPI1;
  spi1_h.Init.Mode = SPI_MODE_MASTER;
  spi1_h.Init.Direction = SPI_DIRECTION_2LINES;
  spi1_h.Init.DataSize = SPI_DATASIZE_8BIT;
  spi1_h.Init.CLKPolarity = SPI_POLARITY_LOW;
  spi1_h.Init.CLKPhase = SPI_PHASE_1EDGE;
  spi1_h.Init.NSS = SPI_NSS_SOFT;
  spi1_h.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  spi1_h.Init.FirstBit = SPI_FIRSTBIT_MSB;
  spi1_h.Init.TIMode = SPI_TIMODE_DISABLED;
  spi1_h.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  return HAL_SPI_Init( &spi1_h );
}

void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi )
{
  GPIO_InitTypeDef gio;
  if( hspi->Instance == SPI1 ) {
    __SPI1_CLK_ENABLE();
    // SPI1 GPIO Configuration
    // A5 --> SPI1_SCK
    // A6 --> SPI1_MISO
    // A7 --> SPI1_MOSI
    // A4 --> manual NSS (separate init)

    gio.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_HIGH;
    gio.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init( GPIOA, &gio );
  }
}

void HAL_SPI_MspDeInit( SPI_HandleTypeDef* hspi )
{
  if( hspi->Instance==SPI1 ) {
    __SPI1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7 );
  }
}

