#include <oxc_base.h>

DAC_HandleTypeDef hdac;

int MX_DAC_Init()
{
  hdac.Instance = DAC;
  if( HAL_DAC_Init(&hdac) != HAL_OK ) {
    return 1;
  }

  DAC_ChannelConfTypeDef sConfig;
  // sConfig.DAC_Trigger   = DAC_TRIGGER_SOFTWARE;
  sConfig.DAC_Trigger      = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_DISABLE;
  if( HAL_DAC_ConfigChannel( &hdac, &sConfig, DAC_CHANNEL_1 ) != HAL_OK ) {
    return 2;
  }


  if( HAL_DAC_ConfigChannel( &hdac, &sConfig, DAC_CHANNEL_1 ) != HAL_OK ) {
    return 3;
  }
  // if( HAL_DACEx_TriangleWaveGenerate( &hdac, DAC_CHANNEL_1, DAC_TRIANGLEAMPLITUDE_127 ) != HAL_OK ) {
  //   return 3;
  // }

  if( HAL_DAC_ConfigChannel( &hdac, &sConfig, DAC_CHANNEL_2 ) != HAL_OK ) {
    return 4;
  }
  return 0;
}

void HAL_DAC_MspInit( DAC_HandleTypeDef* /*dacHandle*/ )
{
  __HAL_RCC_DAC_CLK_ENABLE();

  GPIO_InitTypeDef gio;
  gio.Pin  = GPIO_PIN_4 | GPIO_PIN_5;
  gio.Mode = GPIO_MODE_ANALOG;
  gio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init( GPIOA, &gio );

  // HAL_NVIC_SetPriority( TIM6_DAC_IRQn, 5, 0 );
  // HAL_NVIC_EnableIRQ( TIM6_DAC_IRQn );
}

void HAL_DAC_MspDeInit( DAC_HandleTypeDef* /*dacHandle*/ )
{
  __HAL_RCC_DAC_CLK_DISABLE();
  HAL_GPIO_DeInit( GPIOA, GPIO_PIN_4 | GPIO_PIN_5 );
  // HAL_NVIC_DisableIRQ( TIM6_DAC_IRQn );
}

