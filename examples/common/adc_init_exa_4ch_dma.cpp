#include <errno.h>

#include <oxc_gpio.h>
#include <oxc_adc.h>


void ADC_DMA_reinit( ADC_Info &adc );

int adc_init_exa_4ch_dma( ADC_Info &adc, uint32_t presc, uint32_t sampl_cycl, uint8_t n_ch )
{
  BOARD_ADC_DEFAULT_EN;
  #if defined(STM32F7)
  __HAL_RCC_DAC_CLK_ENABLE(); // !!!!!!!!!!!!! see errata - need for timer interaction
  #endif
  if( n_ch > 4 ) { n_ch = 4; }
  if( n_ch < 1 ) { n_ch = 1; }

  adc.hadc.Instance                   = BOARD_ADC_DEFAULT_DEV;
  adc.hadc.Init.ClockPrescaler        = presc;
  adc.adc_clk                         = calc_ADC_clk( presc, nullptr );
  adc.hadc.Init.Resolution            = ADC_RESOLUTION_12B;
  adc.hadc.Init.ScanConvMode          = ENABLE;  // if disabled, only first channel works
  adc.hadc.Init.ContinuousConvMode    = DISABLE; // to start at trigger or software
  adc.hadc.Init.DiscontinuousConvMode = DISABLE; // if enabled, seems to not work at all
  adc.hadc.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING; // or _NONE
  adc.hadc.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T2_TRGO;

  adc.hadc.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  adc.hadc.Init.NbrOfConversion       = n_ch;
  adc.hadc.Init.DMAContinuousRequests = DISABLE;
  // adc.hadc.Init.DMAContinuousRequests = ENABLE; // for double-buffer DMA?
  // adc.hadc.Init.EOCSelection          = ADC_EOC_SINGLE_CONV; // test: multiple errors after conversion
  adc.hadc.Init.EOCSelection          = ADC_EOC_SEQ_CONV;
  if( HAL_ADC_Init( &adc.hadc ) != HAL_OK ) {
    errno = 3000;
    return 0;
  }
  ADC_DMA_reinit( adc );

  decltype(ADC_CHANNEL_0) static const constexpr chs[] {
    BOARD_ADC_DEFAULT_CH0,
    BOARD_ADC_DEFAULT_CH1,
    BOARD_ADC_DEFAULT_CH2,
    BOARD_ADC_DEFAULT_CH3
  };
  ADC_ChannelConfTypeDef sConfig;
  sConfig.SamplingTime = sampl_cycl;

  int rank = 1;
  for( auto ch : chs  ) {
    sConfig.Channel = ch;
    sConfig.Rank = rank++;
    if( HAL_ADC_ConfigChannel( &adc.hadc , &sConfig ) != HAL_OK )  {
      errno = 3001;
      return 0;
    }
  }
  return 1;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  GPIO_InitTypeDef gio;
  if( adcHandle->Instance == BOARD_ADC_DEFAULT_DEV ) {
    BOARD_ADC_DEFAULT_EN;

    gio.Mode = GPIO_MODE_ANALOG;
    gio.Pull = GPIO_NOPULL;

    GPIO_enableClk( BOARD_ADC_DEFAULT_GPIO0 );
    GPIO_enableClk( BOARD_ADC_DEFAULT_GPIO1 );
    GPIO_enableClk( BOARD_ADC_DEFAULT_GPIO2 );
    GPIO_enableClk( BOARD_ADC_DEFAULT_GPIO3 );
    gio.Pin  = BOARD_ADC_DEFAULT_PIN0;
    HAL_GPIO_Init( BOARD_ADC_DEFAULT_GPIO0, &gio );
    gio.Pin  = BOARD_ADC_DEFAULT_PIN1;
    HAL_GPIO_Init( BOARD_ADC_DEFAULT_GPIO1, &gio );
    gio.Pin  = BOARD_ADC_DEFAULT_PIN2;
    HAL_GPIO_Init( BOARD_ADC_DEFAULT_GPIO2, &gio );
    gio.Pin  = BOARD_ADC_DEFAULT_PIN3;
    HAL_GPIO_Init( BOARD_ADC_DEFAULT_GPIO3, &gio );

    __HAL_RCC_DMA2_CLK_ENABLE();

    // ADC_DMA_reinit();

    HAL_NVIC_SetPriority( DMA2_Stream0_IRQn, 1, 0 );
    HAL_NVIC_EnableIRQ( DMA2_Stream0_IRQn );

    HAL_NVIC_SetPriority( ADC_IRQn, 2, 0 );
    // HAL_NVIC_EnableIRQ( ADC_IRQn );
  }
}

void ADC_DMA_reinit( ADC_Info &adc )
{
  adc.hdma_adc.Instance                 = DMA2_Stream0;
  adc.hdma_adc.Init.Channel             = DMA_CHANNEL_0;
  adc.hdma_adc.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  adc.hdma_adc.Init.PeriphInc           = DMA_PINC_DISABLE;
  adc.hdma_adc.Init.MemInc              = DMA_MINC_ENABLE;
  adc.hdma_adc.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  adc.hdma_adc.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  adc.hdma_adc.Init.Mode                = DMA_NORMAL; // DMA_CIRCULAR, DMA_PFCTRL
  adc.hdma_adc.Init.Priority            = DMA_PRIORITY_HIGH; // DMA_PRIORITY_LOW, DMA_PRIORITY_MEDIUM, DMA_PRIORITY_HIGH, DMA_PRIORITY_VERY_HIGH
  adc.hdma_adc.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  // adc.hdma_adc.Init.FIFOThreshold    = DMA_FIFO_THRESHOLD_HALFFULL;
  // adc.hdma_adc.Init.MemBurst         = DMA_MBURST_SINGLE;
  // adc.hdma_adc.Init.PeriphBurst      = DMA_PBURST_SINGLE;
  if( HAL_DMA_Init( &adc.hdma_adc ) != HAL_OK ) {
    Error_Handler( 6 );
  }

  __HAL_LINKDMA( &adc.hadc, DMA_Handle, adc.hdma_adc );
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == BOARD_ADC_DEFAULT_DEV ) {
    BOARD_ADC_DEFAULT_DIS;
    HAL_GPIO_DeInit( BOARD_ADC_DEFAULT_GPIO0, BOARD_ADC_DEFAULT_PIN0 );
    HAL_GPIO_DeInit( BOARD_ADC_DEFAULT_GPIO1, BOARD_ADC_DEFAULT_PIN1 );
    HAL_GPIO_DeInit( BOARD_ADC_DEFAULT_GPIO2, BOARD_ADC_DEFAULT_PIN2 );
    HAL_GPIO_DeInit( BOARD_ADC_DEFAULT_GPIO3, BOARD_ADC_DEFAULT_PIN3 );
    HAL_DMA_DeInit( adcHandle->DMA_Handle );
  }
}

// not used in single DMA
// void ADC_IRQHandler(void)
// {
//   HAL_ADC_IRQHandler( &adc.hadc );
//   // leds.toggle( BIT0 );
// }



