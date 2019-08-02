#include <errno.h>
#include <oxc_gpio.h>

extern ADC_HandleTypeDef hadc1;

extern uint32_t adc_clk;
void ADC_DMA_REINIT();
uint32_t calc_ADC_clk( uint32_t presc, int *div_val );
uint32_t hint_ADC_presc();

int adc_init_exa_4ch_irq( uint32_t presc, uint32_t sampl_cycl, uint8_t n_ch )
{;
  #if defined(STM32F7)
  __HAL_RCC_DAC_CLK_ENABLE(); // !!!!!!!!!!!!! see errata - need for timer interaction
  #endif

  if( n_ch > 4 ) { n_ch = 4; }
  if( n_ch < 1 ) { n_ch = 1; }

  hadc1.Instance                   = BOARD_ADC_DEFAULT_DEV;
  hadc1.Init.ClockPrescaler        = presc;
  adc_clk                          = calc_ADC_clk( presc, nullptr );
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = ENABLE;  // if disabled, only first channel works
  hadc1.Init.ContinuousConvMode    = DISABLE; // to start at trigger
  hadc1.Init.DiscontinuousConvMode = DISABLE; // if enabled, seems to not work at all
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T2_TRGO;

  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = n_ch;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  // hadc1.Init.DMAContinuousRequests = ENABLE; // for double-buffer DMA?
  // hadc1.Init.EOCSelection          = ADC_EOC_SEQ_CONV; //
  hadc1.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK )  {
    errno = 3000;
    return 0;
  }

  decltype(ADC_CHANNEL_0) static const constexpr chs[] {
    BOARD_ADC_DEFAULT_CH0,
    BOARD_ADC_DEFAULT_CH1,
    BOARD_ADC_DEFAULT_CH2,
    BOARD_ADC_DEFAULT_CH3
  };
  ADC_ChannelConfTypeDef sConfig;
  sConfig.SamplingTime = sampl_cycl;

  int rank = 1;
  for( auto ch : chs  ) {
    sConfig.Channel = ch;
    sConfig.Rank = rank++;
    if( HAL_ADC_ConfigChannel( &hadc1 , &sConfig ) != HAL_OK )  {
      errno = 3001;
      return 0;
    }
  }
  return 1;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == BOARD_ADC_DEFAULT_DEV ) {

    BOARD_ADC_DEFAULT_EN;

    BOARD_ADC_DEFAULT_GPIO0.enableClk();
    BOARD_ADC_DEFAULT_GPIO1.enableClk();
    BOARD_ADC_DEFAULT_GPIO2.enableClk();
    BOARD_ADC_DEFAULT_GPIO3.enableClk();

    BOARD_ADC_DEFAULT_GPIO0.cfgAnalog( BOARD_ADC_DEFAULT_PIN0 );
    BOARD_ADC_DEFAULT_GPIO1.cfgAnalog( BOARD_ADC_DEFAULT_PIN1 );
    BOARD_ADC_DEFAULT_GPIO2.cfgAnalog( BOARD_ADC_DEFAULT_PIN2 );
    BOARD_ADC_DEFAULT_GPIO3.cfgAnalog( BOARD_ADC_DEFAULT_PIN3 );

    HAL_NVIC_SetPriority( ADC_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( ADC_IRQn );
  }
}


void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == BOARD_ADC_DEFAULT_DEV ) {
    BOARD_ADC_DEFAULT_DIS;
    HAL_DMA_DeInit( adcHandle->DMA_Handle );
  }
}


void ADC_IRQHandler(void)
{
  HAL_ADC_IRQHandler( &hadc1 );
  // leds.toggle( LED_BSP_ERR );
}


