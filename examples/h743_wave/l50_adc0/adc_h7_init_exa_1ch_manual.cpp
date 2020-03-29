#include <errno.h>
#include <oxc_gpio.h>

extern ADC_HandleTypeDef hadc1;

int adc_h7_init_exa_1ch_manual( uint32_t presc, uint32_t sampl_cycl )
{
  BOARD_ADC_DEFAULT_EN;

  hadc1.Instance                      = BOARD_ADC_DEFAULT_DEV;
  hadc1.Init.ClockPrescaler           = presc; // ADC_CLOCK_ASYNC_DIV4;
  hadc1.Init.Resolution               = ADC_RESOLUTION_16B;
  hadc1.Init.ScanConvMode             = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait         = DISABLE;
  hadc1.Init.ContinuousConvMode       = DISABLE;
  hadc1.Init.NbrOfConversion          = 1;
  hadc1.Init.DiscontinuousConvMode    = DISABLE;
  hadc1.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
  hadc1.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.OversamplingMode         = DISABLE;

  // hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK ) {
    errno = 3000;
    return 0;
  }

  //* no ADC multi-mode
  ADC_MultiModeTypeDef multimode;
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if( HAL_ADCEx_MultiModeConfigChannel( &hadc1, &multimode ) != HAL_OK ) {
    errno = 3040;
    return 0;
  }

  ADC_ChannelConfTypeDef sConfig;
  sConfig.Channel      = BOARD_ADC_DEFAULT_CH0;
  sConfig.Rank         = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = sampl_cycl;
  sConfig.SingleDiff   = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset       = 0;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK )  {
    errno = 3001;
    return 0;
  }

  // H7 only?
  if( HAL_ADCEx_Calibration_Start( &hadc1, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED ) != HAL_OK ) {
    errno = 3041;
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

