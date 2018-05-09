#include <oxc_auto.h>

SD_HandleTypeDef hsd;
DMA_HandleTypeDef hdma_sdio_rx;
DMA_HandleTypeDef hdma_sdio_tx;

#ifdef SDIO
 #define SDIO_X SDIO
 #define SDIO_CLOCK_X   SDIO_CLOCK_EDGE_RISING
 #define SDIO_BYPASS_X  SDIO_CLOCK_POWER_SAVE_DISABLE
 #define SDIO_PS_X      SDIO_CLOCK_POWER_SAVE_DISABLE
 #define SDIO_BUS_X     SDIO_BUS_WIDE_1B
 #define SDIO_FLOW_X    SDIO_HARDWARE_FLOW_CONTROL_DISABLE
#else
 #define SDIO_X SDMMC1
 #define SDIO_CLOCK_X   SDMMC_CLOCK_EDGE_RISING
 #define SDIO_BYPASS_X  SDMMC_CLOCK_BYPASS_DISABLE
 #define SDIO_PS_X      SDMMC_CLOCK_POWER_SAVE_DISABLE
 #define SDIO_BUS_X     SDMMC_BUS_WIDE_1B
 #define SDIO_FLOW_X    SDMMC_HARDWARE_FLOW_CONTROL_DISABLE
#endif

void MX_SDIO_SD_Init()
{
  hsd.Instance                 = SDIO_X;
  hsd.Init.ClockEdge           = SDIO_CLOCK_X;
  hsd.Init.ClockBypass         = SDIO_BYPASS_X;
  hsd.Init.ClockPowerSave      = SDIO_PS_X;
  hsd.Init.BusWide             = SDIO_BUS_X;
  hsd.Init.HardwareFlowControl = SDIO_FLOW_X;
  hsd.Init.ClockDiv            = 8; // 200?
}

void HAL_SD_MspInit( SD_HandleTypeDef* sdHandle )
{
  // no check: the only SDIO
  SD_EXA_CLKEN;

  GPIO_InitTypeDef gio;
  gio.Pin       = SD_EXA_CK_PIN;
  gio.Mode      = GPIO_MODE_AF_PP;
  gio.Pull      = GPIO_NOPULL;
  gio.Speed     = GPIO_SPEED_MAX;
  gio.Alternate = SD_EXA_GPIOAF;
  HAL_GPIO_Init( SD_EXA_CK_GPIO, &gio );

  gio.Pin       = SD_EXA_D0_PIN;
  HAL_GPIO_Init( SD_EXA_D0_GPIO, &gio );

  gio.Pin       = SD_EXA_CMD_PIN;
  HAL_GPIO_Init( SD_EXA_CMD_GPIO, &gio );

  // DMA init: from dma.c: MX_DMA_Init
  __HAL_RCC_DMA2_CLK_ENABLE();
  /* DMA2_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( DMA2_Stream3_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 ); // TODO: 5-> ???
  HAL_NVIC_EnableIRQ( DMA2_Stream3_IRQn );
  /* DMA2_Stream6_IRQn interrupt configuration */
  HAL_NVIC_SetPriority( DMA2_Stream6_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( DMA2_Stream6_IRQn );
  // end MX_DMA_Init

  hdma_sdio_rx.Instance                 = DMA2_Stream3;
  hdma_sdio_rx.Init.Channel             = DMA_CHANNEL_4;
  hdma_sdio_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_sdio_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_sdio_rx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_sdio_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_sdio_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
  hdma_sdio_rx.Init.Mode                = DMA_PFCTRL;
  hdma_sdio_rx.Init.Priority            = DMA_PRIORITY_LOW;
  hdma_sdio_rx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
  hdma_sdio_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_sdio_rx.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_sdio_rx.Init.PeriphBurst         = DMA_PBURST_INC4;
  if( HAL_DMA_Init( &hdma_sdio_rx ) != HAL_OK ) {
    // _Error_Handler(__FILE__, __LINE__);
  }
  __HAL_LINKDMA( sdHandle, hdmarx, hdma_sdio_rx );

  /* SDIO_TX Init */
  hdma_sdio_tx.Instance                 = DMA2_Stream6;
  hdma_sdio_tx.Init.Channel             = DMA_CHANNEL_4;
  hdma_sdio_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_sdio_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_sdio_tx.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_sdio_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma_sdio_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
  hdma_sdio_tx.Init.Mode                = DMA_PFCTRL;
  hdma_sdio_tx.Init.Priority            = DMA_PRIORITY_LOW;
  hdma_sdio_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
  hdma_sdio_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_sdio_tx.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_sdio_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
  if( HAL_DMA_Init( &hdma_sdio_tx ) != HAL_OK ) {
    // _Error_Handler(__FILE__, __LINE__);
  }

  __HAL_LINKDMA( sdHandle, hdmatx, hdma_sdio_tx );

  /* SDIO interrupt Init */
  HAL_NVIC_SetPriority( SDIO_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
  HAL_NVIC_EnableIRQ( SDIO_IRQn );

}

void HAL_SD_MspDeInit( SD_HandleTypeDef *sdHandle )
{
  HAL_DMA_DeInit( sdHandle->hdmarx );
  HAL_DMA_DeInit( sdHandle->hdmatx );

  HAL_NVIC_DisableIRQ( SDIO_IRQn );

  SD_EXA_CLKDIS;
  HAL_GPIO_DeInit( SD_EXA_CK_GPIO,  SD_EXA_CK_PIN  );
  HAL_GPIO_DeInit( SD_EXA_D0_GPIO,  SD_EXA_D0_PIN  );
  HAL_GPIO_DeInit( SD_EXA_CMD_GPIO, SD_EXA_CMD_PIN );
}

