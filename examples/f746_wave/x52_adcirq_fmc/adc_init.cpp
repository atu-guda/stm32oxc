#include <oxc_base.h>
#include <oxc_gpio.h>

extern ADC_HandleTypeDef hadc1;
//ADC_HandleTypeDef hadc2;
//ADC_HandleTypeDef hadc3;
// extern DMA_HandleTypeDef hdma_adc1;


void MX_ADC1_Init( uint8_t n_ch, uint32_t sampl_time )
{
  __HAL_RCC_ADC1_CLK_ENABLE();
  __HAL_RCC_DAC_CLK_ENABLE(); // !!!!!!!!!!!!! ?????????? see errata
  ADC_ChannelConfTypeDef sConfig;
  if( n_ch > 4 ) { n_ch = 4; }
  if( n_ch < 1 ) { n_ch = 1; }

  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4; // 100/4 = 25 Mhz, max 36
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = ENABLE;  // if disabled, only first channel works
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
  decltype(ADC_CHANNEL_0) static const constexpr chs[]  { ADC_CHANNEL_0, ADC_CHANNEL_1, ADC_CHANNEL_2, ADC_CHANNEL_4 }; // A4: TMP
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
  if( adcHandle->Instance == ADC1 )  {
    __HAL_RCC_ADC1_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();

    //* ADC1 GPIO Configuration        A0-A3   ------> ADC1: IN0-IN2, IN4 // A4: tmp
    GpioA.cfgAnalog_N(  GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 |  GPIO_PIN_4 );

    HAL_NVIC_SetPriority( ADC_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( ADC_IRQn );

  }
}


void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == ADC1 )  {
    __HAL_RCC_ADC1_CLK_DISABLE();
  }

}

void ADC_IRQHandler(void)
{
  HAL_ADC_IRQHandler( &hadc1 );
  // leds.toggle( BIT0 );
}


