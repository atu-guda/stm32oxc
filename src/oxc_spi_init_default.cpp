#include <oxc_auto.h>

// --------------------------- SPI --------------------------------------

extern SPI_HandleTypeDef spi_h; // in main.c

int SPI_init_default( uint32_t baud_presc, SPI_lmode::lmode_enum lmode /* = SPI_lmode::low_1e */ )
{
  spi_h.Instance               = BOARD_SPI_DEFAULT;
  spi_h.Init.Mode              = SPI_MODE_MASTER;
  spi_h.Init.Direction         = SPI_DIRECTION_2LINES;
  spi_h.Init.DataSize          = SPI_DATASIZE_8BIT;
  spi_h.Init.CLKPolarity       = ( lmode & SPI_lmode::high_1e ) ? SPI_POLARITY_HIGH : SPI_POLARITY_LOW;
  spi_h.Init.CLKPhase          = ( lmode & SPI_lmode::low_2e  ) ? SPI_PHASE_2EDGE   : SPI_PHASE_1EDGE;
  spi_h.Init.NSS               = SPI_NSS_SOFT;
  spi_h.Init.BaudRatePrescaler = baud_presc; // SPI_BAUDRATEPRESCALER_2; ... _256
  spi_h.Init.FirstBit          = SPI_FIRSTBIT_MSB;
  spi_h.Init.TIMode            = SPI_TIMODE_DISABLED;
  spi_h.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLED;
  return HAL_SPI_Init( &spi_h );
}

void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi )
{
  GPIO_InitTypeDef gio;
  if( hspi->Instance == BOARD_SPI_DEFAULT ) {
    BOARD_SPI_DEFAULT_ENABLE;

    gio.Mode      = GPIO_MODE_AF_PP;
    gio.Pull      = GPIO_NOPULL;
    gio.Speed     = GPIO_SPEED_MAX;

    #if  ! defined (STM32F1)
    gio.Alternate = BOARD_SPI_DEFAULT_GPIO_AF;
    #endif

    gio.Pin       = BOARD_SPI_DEFAULT_GPIO_PIN_SCK;
    HAL_GPIO_Init( BOARD_SPI_DEFAULT_GPIO_SCK, &gio );
    gio.Pin       = BOARD_SPI_DEFAULT_GPIO_PIN_MISO;
    HAL_GPIO_Init( BOARD_SPI_DEFAULT_GPIO_MISO, &gio );
    gio.Pin       = BOARD_SPI_DEFAULT_GPIO_PIN_MOSI;
    HAL_GPIO_Init( BOARD_SPI_DEFAULT_GPIO_MOSI, &gio );
  }
}

void HAL_SPI_MspDeInit( SPI_HandleTypeDef* hspi )
{
  if( hspi->Instance == BOARD_SPI_DEFAULT ) {
    BOARD_SPI_DEFAULT_DISABLE;
    HAL_GPIO_DeInit( BOARD_SPI_DEFAULT_GPIO_SCK,  BOARD_SPI_DEFAULT_GPIO_PIN_SCK  );
    HAL_GPIO_DeInit( BOARD_SPI_DEFAULT_GPIO_MISO, BOARD_SPI_DEFAULT_GPIO_PIN_MISO );
    HAL_GPIO_DeInit( BOARD_SPI_DEFAULT_GPIO_MOSI, BOARD_SPI_DEFAULT_GPIO_PIN_MOSI );
  }
}

