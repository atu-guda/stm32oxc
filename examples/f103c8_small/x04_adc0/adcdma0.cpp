#include <oxc_auto.h>

DMA_HandleTypeDef hdma_adc1;
ADC_HandleTypeDef hadc1;

const uint32_t adc_cycles = ADC_SAMPLETIME_55CYCLES_5;

void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig;

  hadc1.Instance                   = ADC1;
  hadc1.Init.ScanConvMode          = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 4;
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK )   {
    die4led( 1 );
  }

  sConfig.Channel = ADC_CHANNEL_0;  sConfig.Rank = 1;
  sConfig.SamplingTime = adc_cycles;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK )   {
    die4led( 2 );
  }
  sConfig.Channel = ADC_CHANNEL_1;  sConfig.Rank = 2;
  HAL_ADC_ConfigChannel( &hadc1, &sConfig );
  sConfig.Channel = ADC_CHANNEL_2;  sConfig.Rank = 3;
  HAL_ADC_ConfigChannel( &hadc1, &sConfig );
  sConfig.Channel = ADC_CHANNEL_3;  sConfig.Rank = 4;
  HAL_ADC_ConfigChannel( &hadc1, &sConfig );
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  GPIO_InitTypeDef gio;
  if( adcHandle->Instance == ADC1 ) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    // A0:A3 : ADC1_IN0:IN3
    GpioA.cfgAnalog_N( GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 );

    hdma_adc1.Instance                 = DMA1_Channel1;
    hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma_adc1.Init.Mode                = DMA_NORMAL;
    hdma_adc1.Init.Priority            = DMA_PRIORITY_HIGH;
    if( HAL_DMA_Init( &hdma_adc1 ) != HAL_OK ) {
      die4led( 0x04 );
    }

    __HAL_LINKDMA( adcHandle, DMA_Handle, hdma_adc1 );
  }
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == ADC1 ) {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_DMA_DeInit( adcHandle->DMA_Handle );
  }
}

void MX_DMA_Init()
{
  __HAL_RCC_DMA1_CLK_ENABLE();
  HAL_NVIC_SetPriority( DMA1_Channel1_IRQn, 2, 0 );
  HAL_NVIC_EnableIRQ( DMA1_Channel1_IRQn );
}


void DMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler( &hdma_adc1 );
}

