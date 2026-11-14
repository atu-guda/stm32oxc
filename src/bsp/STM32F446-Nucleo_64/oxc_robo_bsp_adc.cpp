#include <oxc_auto.h>

#include <board_robo_cfg.h>

using namespace oxc;

ADC_HandleTypeDef hadc_sensor;
DMA_HandleTypeDef hdma_adc_sensor;
// ADC_Info adc_sensor ( ADC_SENSOR, ADC_SENSOR_CHPINS );

ReturnCode adc_sensor_init()
{
  adc_sensor.prepare_multi_ev( ADC_SENSOR_N_CH, ADC_SENSOR_ClockPrescaler, ADC_SENSOR_SAMPLETIME,
      ADC_SOFTWARE_START, ADC_SENSOR_Resolution );
  adc_sensor.init_common();
  return rcOk;
}


void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != ADC_SENSOR ) {
    return;
  }
  ADC_SENSOR_CLK_EN();
  ADC_SENSOR_DMA_CLK_EN();

  adc_sensor.init_gpio_channels();
  adc_sensor.DMA_reinit( DMA_NORMAL );

  HAL_NVIC_SetPriority( ADC_SENSOR_DMA_IRQ, ADC_SENSOR_DMA_IRQ_PRTY, 0 );
  HAL_NVIC_EnableIRQ(   ADC_SENSOR_DMA_IRQ );
  // UVAR_x = 1;
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance != ADC_SENSOR ) {
    return;
  }
  ADC_SENSOR_CLK_DIS();
  ADC_SENSOR_DMA_CLK_DIS();
}

void HAL_ADC_ConvHalfCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds[1].set();
  // adc_sensor.convHalfCpltCallback( hadc );
}


void HAL_ADC_ConvCpltCallback( ADC_HandleTypeDef *hadc )
{
  // leds[1].set();
  // ++UVAR_i;
  adc_sensor.convCpltCallback( hadc );
}

void HAL_ADC_ErrorCallback( ADC_HandleTypeDef *hadc )
{
  // leds[0].set();
  // ++UVAR_j;
  adc_sensor.errorCallback( hadc );
}

void BOARD_ADC_DMA_IRQHANDLER(void)
{
  // leds[0].set();
  // ++UVAR_k;
  HAL_DMA_IRQHandler( &adc_sensor.hdma_adc );
}




