#include <oxc_base.h>

extern ADC_HandleTypeDef hadc1;
//ADC_HandleTypeDef hadc2;
//ADC_HandleTypeDef hadc3;
extern DMA_HandleTypeDef hdma_adc1;
extern uint32_t adc_clk;

void ADC_DMA_REINIT();

void MX_ADC1_Init( uint8_t n_ch, uint32_t sampl_time )
{
  __HAL_RCC_ADC1_CLK_ENABLE();
  ADC_ChannelConfTypeDef sConfig;
  if( n_ch > 4 ) { n_ch = 4; }
  if( n_ch < 1 ) { n_ch = 1; }

  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV2; // 72/2 = 36 MHz  = MAX
  adc_clk = 36000000;
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = ENABLE;  // if disabled, only first channel works
  // hadc1.Init.ScanConvMode          = DISABLE;
  // hadc1.Init.ContinuousConvMode    = ENABLE;  // if disabled, only first 1 on NbrOfConversion received
  hadc1.Init.ContinuousConvMode    = DISABLE; // to start at trigger
  hadc1.Init.DiscontinuousConvMode = DISABLE; // if enabled, seems to not work at all
  // hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T2_TRGO;

  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = n_ch;
  hadc1.Init.DMAContinuousRequests = DISABLE; // ???
  /// hadc1.Init.DMAContinuousRequests = ENABLE; // ???
  // hadc1.Init.EOCSelection          = ADC_EOC_SEQ_CONV; // ADC_EOC_SINGLE_CONV; // seems dont metter if DMA
  hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV; // test
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK )  {
    Error_Handler( 2 );
  }

  /**Configure for the selected ADC regular channels its corresponding rank in the sequencer and its sample time.  */
  decltype(ADC_CHANNEL_0) static const constexpr chs[]  { ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_6, ADC_CHANNEL_7 };
  sConfig.SamplingTime = sampl_time;

  int rank = 1;
  for( auto ch : chs  ) {
    sConfig.Channel = ch;
    sConfig.Rank = rank++;
    if( HAL_ADC_ConfigChannel( &hadc1 , &sConfig ) != HAL_OK )  {
      Error_Handler( 3 );
    }
  }

}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if( adcHandle->Instance == ADC1 )  {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();

    //* ADC1 GPIO Configuration        PA1   ------> ADC1_IN1....
    GPIO_InitStruct.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_6  | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    // DMA part
    __HAL_RCC_DMA2_CLK_ENABLE();


    ADC_DMA_REINIT();

    // HAL_NVIC_SetPriority( DMA2_Stream0_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
    HAL_NVIC_SetPriority( DMA2_Stream0_IRQn, 1, 0 );
    HAL_NVIC_EnableIRQ( DMA2_Stream0_IRQn );

    HAL_NVIC_SetPriority( ADC_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( ADC_IRQn );

  }
}

void ADC_DMA_REINIT()
{
    // #<{(| Peripheral DMA init|)}>#
    hdma_adc1.Instance                 = DMA2_Stream0;
    hdma_adc1.Init.Channel             = DMA_CHANNEL_0;
    hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode                = DMA_NORMAL;
    hdma_adc1.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if( HAL_DMA_Init( &hdma_adc1 ) != HAL_OK )   {
      Error_Handler( 6 );
    }

    __HAL_LINKDMA( &hadc1, DMA_Handle, hdma_adc1 );
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == ADC1 )  {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_6  | GPIO_PIN_7 );
    HAL_DMA_DeInit( adcHandle->DMA_Handle );
  }

}

void ADC_IRQHandler(void)
{
  HAL_ADC_IRQHandler( &hadc1 );
  // leds.toggle( BIT0 );
}

void DMA2_Stream0_IRQHandler(void)
{
  HAL_DMA_IRQHandler( &hdma_adc1 );
}


