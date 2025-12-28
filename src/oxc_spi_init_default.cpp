#include <cstring>
#include <oxc_auto.h>

// --------------------------- SPI --------------------------------------

extern SPI_HandleTypeDef spi_h; // in main.c

int SPI_init_default( uint32_t baud_presc, SPI_lmode::lmode_enum lmode /* = SPI_lmode::low_1e */ )
{
  spi_h.Instance               = BOARD_SPI_DEFAULT;
  std::memset( &spi_h.Init, '\0', sizeof(spi_h.Init) );
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
#ifdef STM32H7
  spi_h.Init.NSSPMode                   = SPI_NSS_PULSE_DISABLE;
  // spi_h.Init.MasterKeepIOState       = SPI_MASTER_KEEP_IO_STATE_ENABLE;
  spi_h.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  spi_h.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  spi_h.Init.MasterSSIdleness           = SPI_MASTER_SS_IDLENESS_00CYCLE;
  spi_h.Init.MasterInterDataIdleness    = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  spi_h.Init.MasterReceiverAutoSusp     = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  spi_h.Init.MasterKeepIOState          = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  spi_h.Init.IOSwap                     = SPI_IO_SWAP_DISABLE;
#endif
  return HAL_SPI_Init( &spi_h );
}

void HAL_SPI_MspInit( SPI_HandleTypeDef *hspi )
{
  if( hspi->Instance != BOARD_SPI_DEFAULT ) {
    return;
  }
  BOARD_SPI_DEFAULT_ENABLE;

  BOARD_SPI_DEFAULT_PIN_SCK.cfgAF(  BOARD_SPI_DEFAULT_GPIO_AF );
  BOARD_SPI_DEFAULT_PIN_MISO.cfgAF( BOARD_SPI_DEFAULT_GPIO_AF );
  BOARD_SPI_DEFAULT_PIN_MOSI.cfgAF( BOARD_SPI_DEFAULT_GPIO_AF );
}

void HAL_SPI_MspDeInit( SPI_HandleTypeDef* hspi )
{
  if( hspi->Instance == BOARD_SPI_DEFAULT ) {
    BOARD_SPI_DEFAULT_DISABLE;
    // BOARD_SPI_DEFAULT_GPIO_SCK.cfgIn(  BOARD_SPI_DEFAULT_GPIO_PIN_SCK  );
    // BOARD_SPI_DEFAULT_GPIO_MISO.cfgIn( BOARD_SPI_DEFAULT_GPIO_PIN_MISO );
    // BOARD_SPI_DEFAULT_GPIO_MOSI.cfgIn( BOARD_SPI_DEFAULT_GPIO_PIN_MOSI );
  }
}

