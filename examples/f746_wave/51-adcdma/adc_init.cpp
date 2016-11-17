#include <oxc_base.h>
#include <oxc_gpio.h>

extern ADC_HandleTypeDef hadc1;
//ADC_HandleTypeDef hadc2;
//ADC_HandleTypeDef hadc3;
extern DMA_HandleTypeDef hdma_adc1;

void ADC_DMA_REINIT();

void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;

  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4; // 100/4 = 25 Mhz, max 36
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = ENABLE;  // if disabled, only first channel works
  hadc1.Init.ContinuousConvMode    = ENABLE;  // if disabled, only first 1 on NbrOfConversion received
  hadc1.Init.DiscontinuousConvMode = DISABLE; // if enabled, seems to not work at all
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 4;
  hadc1.Init.DMAContinuousRequests = DISABLE; // ???
  hadc1.Init.EOCSelection          = ADC_EOC_SEQ_CONV; // ADC_EOC_SINGLE_CONV; // seens dont metter if DMA
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK )  {
    Error_Handler( 2 );
  }

  /**Configure for the selected ADC regular channels its corresponding rank in the sequencer and its sample time.  */
  decltype(ADC_CHANNEL_0) static const constexpr chs[]  { ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_3 };
  // sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;   //  15  tick: 1.6 MSa,  0.6  us
  sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;  //  27  tick: 925 kSa,  1.08 us
  // sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES;  //  40  tick: 615 kSa,  1.6  us
  // sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;  //  68  tick: 367 kSa,  2.72 us
  // sConfig.SamplingTime = ADC_SAMPLETIME_84CYCLES;  //  96  tick: 260 kSa,  3.84 us
  // sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES; // 156  tick: 160 kSa,  6.24 us
  // sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES; // 492  tick:  50 kSa, 19.68 us
  int rank = 1;
  for( auto ch : chs  ) {
    sConfig.Channel = ch;
    sConfig.Rank = rank++;
    if( HAL_ADC_ConfigChannel( &hadc1 , &sConfig ) != HAL_OK )  {
      Error_Handler( 3 );
    }
  }

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(adcHandle->Instance==ADC1)  {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();

    //* ADC1 GPIO Configuration        A0-A3   ------> ADC1: IN0-IN3
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    // DMA part
    __HAL_RCC_DMA2_CLK_ENABLE();


    ADC_DMA_REINIT();

    HAL_NVIC_SetPriority( DMA2_Stream0_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
    HAL_NVIC_EnableIRQ( DMA2_Stream0_IRQn );

    // HAL_NVIC_SetPriority( ADC_IRQn, configKERNEL_INTERRUPT_PRIORITY, 0 );
    // HAL_NVIC_EnableIRQ( ADC_IRQn );

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
    hdma_adc1.Init.Priority            = DMA_PRIORITY_MEDIUM;
    hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    if( HAL_DMA_Init( &hdma_adc1 ) != HAL_OK )   {
      Error_Handler( 6 );
    }

    __HAL_LINKDMA( &hadc1, DMA_Handle, hdma_adc1 );
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance==ADC1 )   {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_1 );
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
  leds.toggle( BIT1 );
}


