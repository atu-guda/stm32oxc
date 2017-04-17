#include <oxc_base.h>

extern ADC_HandleTypeDef hadc1;
//ADC_HandleTypeDef hadc2;
//ADC_HandleTypeDef hadc3;
//DMA_HandleTypeDef hdma_adc1;

void MX_ADC1_Init(void)
{
  __HAL_RCC_ADC1_CLK_ENABLE();
  ADC_ChannelConfTypeDef sConfig;

  /**Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)     */
  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = DISABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK )  {
    Error_Handler( 2 );
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  // sConfig.SamplingTime = ADC_SAMPLETIME_15CYCLES;
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if( HAL_ADC_ConfigChannel( &hadc1 , &sConfig ) != HAL_OK )  {
    Error_Handler( 3 );
  }

}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  if( adcHandle->Instance==ADC1 )  {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();

    //* ADC1 GPIO Configuration        PA1   ------> ADC1_IN1....
    GPIO_InitStruct.Pin  = GPIO_PIN_0; // | GPIO_PIN_1 | GPIO_PIN_6  | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

  }
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == ADC1 )  {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOA, GPIO_PIN_1 );
    // HAL_DMA_DeInit(adcHandle->DMA_Handle);
  }

}

