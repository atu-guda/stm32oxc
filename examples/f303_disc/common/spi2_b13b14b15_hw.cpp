#include <oxc_base.h>


extern SPI_HandleTypeDef spi_h; // in main.c

int MX_SPI2_Init( uint32_t prescal )
{
  spi_h.Instance = SPI2;
  spi_h.Init.Mode = SPI_MODE_MASTER;
  spi_h.Init.Direction = SPI_DIRECTION_2LINES;
  spi_h.Init.DataSize = SPI_DATASIZE_8BIT;
  spi_h.Init.CLKPolarity = SPI_POLARITY_LOW;
  spi_h.Init.CLKPhase = SPI_PHASE_1EDGE;
  spi_h.Init.NSS = SPI_NSS_SOFT;
  spi_h.Init.BaudRatePrescaler = prescal;
  // spi_h.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  spi_h.Init.FirstBit = SPI_FIRSTBIT_MSB;
  spi_h.Init.TIMode = SPI_TIMODE_DISABLED;
  spi_h.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  return HAL_SPI_Init( &spi_h );
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
    gio.Speed = GPIO_SPEED_MAX;
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


