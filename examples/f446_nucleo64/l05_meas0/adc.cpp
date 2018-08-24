#include <oxc_auto.h>

#include "meas0.h"

ADC_HandleTypeDef hadc1;

int MX_ADC1_Init()
{
  ADC_ChannelConfTypeDef sConfig;

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 4;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) {
    return 0;
  }

  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    return 0;
  }

  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    return 0;
  }

  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank = 3;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)  {
    return 0;
  }

  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 4;
  if(HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
    return 0;
  }
  return 1;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  GPIO_InitTypeDef gio;
  if( adcHandle->Instance==ADC1 ) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    /** PC0 --> ADC1_IN10 PC1 --> ADC1_IN11 PC2 --> ADC1_IN12 PC3 --> ADC1_IN13 */
    gio.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    gio.Mode = GPIO_MODE_ANALOG;
    gio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOC, &gio );
  }
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == ADC1 )  {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 );
  }
}

void dac_out( float v0, float v1 )
{
  if( v0 < 0 ) {
    v0 = 0;
  }
  if( v0 > vref  ) {
    v0 = vref;
  }
  if( v1 < 0 ) {
    v1 = 0;
  }
  if( v1 > vref  ) {
    v1 = vref;
  }

  HAL_DAC_SetValue( &hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, (int)( v0 / vref * 4095 ) );
  HAL_DAC_SetValue( &hdac, DAC_CHANNEL_2, DAC_ALIGN_12B_R, (int)( v1 / vref * 4095 ) );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_1 );
  HAL_DAC_Start( &hdac, DAC_CHANNEL_2 );
}

