#include <errno.h>

#include <oxc_adc.h>

int adc_arch_init_exa_1ch_manual( ADC_Info &adc,  uint32_t presc, uint32_t sampl_cycl )
{
  BOARD_ADC_DEFAULT_EN;

  if( adc.hadc.Instance == 0 ) { // must be set beforehand
    errno = 3000;
    return 0;
  }

  adc.hadc.Init.ClockPrescaler           = presc;
  // adc.hadc.Init.Resolution               = BOARD_ADC_DEFAULT_RESOLUTION; // before
  adc.hadc.Init.ScanConvMode             = ADC_SCAN_DISABLE;
  adc.hadc.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
  adc.hadc.Init.LowPowerAutoWait         = DISABLE;
  adc.hadc.Init.ContinuousConvMode       = DISABLE;
  adc.hadc.Init.NbrOfConversion          = 1;
  adc.hadc.Init.DiscontinuousConvMode    = DISABLE;
  adc.hadc.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
  adc.hadc.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
  adc.hadc.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  adc.hadc.Init.Overrun                  = ADC_OVR_DATA_OVERWRITTEN;
  adc.hadc.Init.LeftBitShift             = ADC_LEFTBITSHIFT_NONE;
  adc.hadc.Init.OversamplingMode         = DISABLE;

  if( HAL_ADC_Init( &adc.hadc ) != HAL_OK ) {
    errno = 3001;
    return 0;
  }

  //* no ADC multi-mode
  ADC_MultiModeTypeDef multimode;
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if( HAL_ADCEx_MultiModeConfigChannel( &adc.hadc, &multimode ) != HAL_OK ) {
    errno = 3002;
    return 0;
  }

  if( ! adc.init_adc_channels( sampl_cycl ) )  {
    errno = 3003;
    return 0;
  }

  if( HAL_ADCEx_Calibration_Start( &adc.hadc, ADC_CALIB_OFFSET, ADC_SINGLE_ENDED ) != HAL_OK ) {
    errno = 3010;
    return 0;
  }

  return 1;
}


