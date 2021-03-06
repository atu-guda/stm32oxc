#include <errno.h>
#include <oxc_gpio.h>

extern ADC_HandleTypeDef hadc1;

int adc_init_exa_1ch_manual( uint32_t presc, uint32_t sampl_cycl )
{
  BOARD_ADC_DEFAULT_EN;
  hadc1.Instance                   = BOARD_ADC_DEFAULT_DEV;
  hadc1.Init.ClockPrescaler        = presc;
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = DISABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK ) {
    errno = 3000;
    return 0;
  }

  ADC_ChannelConfTypeDef sConfig;
  sConfig.Channel      = BOARD_ADC_DEFAULT_CH0;
  sConfig.Rank         = 1;
  sConfig.SamplingTime = sampl_cycl; // ADC_SAMPLETIME_15CYCLES;
  if( HAL_ADC_ConfigChannel( &hadc1 , &sConfig ) != HAL_OK )  {
    errno = 3001;
    return 0;
  }
  return 1;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != BOARD_ADC_DEFAULT_DEV ) {
    return;
  }

  BOARD_ADC_DEFAULT_EN;

  BOARD_ADC_DEFAULT_GPIO0.enableClk();
  BOARD_ADC_DEFAULT_GPIO0.cfgAnalog( BOARD_ADC_DEFAULT_PIN0 );
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != BOARD_ADC_DEFAULT_DEV ) {
    return;
  }

  BOARD_ADC_DEFAULT_DIS;
}

