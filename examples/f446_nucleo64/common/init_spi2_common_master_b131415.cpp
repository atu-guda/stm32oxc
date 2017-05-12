#include <oxc_base.h>
#include <oxc_gpio.h>

// --------------------------- SPI --------------------------------------

extern SPI_HandleTypeDef spi_h; // in main.c

int SPI2_Init_common( uint32_t baud_presc )
{
  spi_h.Instance = SPI2;
  spi_h.Init.Mode = SPI_MODE_MASTER;
  spi_h.Init.Direction = SPI_DIRECTION_2LINES;
  spi_h.Init.DataSize = SPI_DATASIZE_8BIT;
  spi_h.Init.CLKPolarity = SPI_POLARITY_LOW;
  spi_h.Init.CLKPhase = SPI_PHASE_1EDGE;
  spi_h.Init.NSS = SPI_NSS_SOFT;
  // spi_h.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; ... _256
  spi_h.Init.BaudRatePrescaler = baud_presc;
  spi_h.Init.FirstBit = SPI_FIRSTBIT_MSB;
  spi_h.Init.TIMode = SPI_TIMODE_DISABLED;
  spi_h.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  return HAL_SPI_Init( &spi_h );
}

void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi )
{
  GPIO_InitTypeDef gio;
  if( hspi->Instance == SPI2 ) {
    __GPIOB_CLK_ENABLE();
    __SPI2_CLK_ENABLE();
    // SPI2 GPIO Configuration
    // B13 --> SPI2_SCK
    // B14 --> SPI2_MISO
    // B15 --> SPI2_MOSI
    // B1  --> manual NSS (separate init)

    gio.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15;
    gio.Mode = GPIO_MODE_AF_PP;
    gio.Pull = GPIO_NOPULL;
    gio.Speed = GPIO_SPEED_MAX;
    gio.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init( GPIOB, &gio );
  }
}

void HAL_SPI_MspDeInit( SPI_HandleTypeDef* hspi )
{
  if( hspi->Instance == SPI2 ) {
    __SPI2_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOB, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 );
  }
}

