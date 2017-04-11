#include <oxc_base.h>
#include <oxc_gpio.h>

// --------------------------- SPI --------------------------------------

extern SPI_HandleTypeDef spi2_h; // in main.c

int SPI2_Init_common( uint32_t baud_presc )
{
  spi2_h.Instance = SPI2;
  spi2_h.Init.Mode = SPI_MODE_MASTER;
  spi2_h.Init.Direction = SPI_DIRECTION_2LINES;
  spi2_h.Init.DataSize = SPI_DATASIZE_8BIT;
  spi2_h.Init.CLKPolarity = SPI_POLARITY_LOW;
  spi2_h.Init.CLKPhase = SPI_PHASE_1EDGE;
  spi2_h.Init.NSS = SPI_NSS_SOFT;
  // spi2_h.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; ... _256
  spi2_h.Init.BaudRatePrescaler = baud_presc;
  spi2_h.Init.FirstBit = SPI_FIRSTBIT_MSB;
  spi2_h.Init.TIMode = SPI_TIMODE_DISABLED;
  spi2_h.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  return HAL_SPI_Init( &spi2_h );
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
    gio.Speed = GPIO_SPEED_HIGH;
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

