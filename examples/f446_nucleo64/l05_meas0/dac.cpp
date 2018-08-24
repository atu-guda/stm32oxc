#include <oxc_auto.h>

#include "meas0.h"

DAC_HandleTypeDef hdac;

int MX_DAC_Init(void)
{
  hdac.Instance = DAC;
  if( HAL_DAC_Init( &hdac ) != HAL_OK ) {
    return 0;
  }

  DAC_ChannelConfTypeDef sConfig;
  sConfig.DAC_Trigger      = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if( HAL_DAC_ConfigChannel( &hdac, &sConfig, DAC_CHANNEL_1 ) != HAL_OK ) {
    return 0;
  }

  if( HAL_DAC_ConfigChannel( &hdac, &sConfig, DAC_CHANNEL_2 ) != HAL_OK ) {
    return 0;
  }
  return 1;
}

void HAL_DAC_MspInit( DAC_HandleTypeDef* dacHandle )
{
  if( dacHandle->Instance != DAC ) {
    return;
  }

  __HAL_RCC_DAC_CLK_ENABLE();
  GPIO_InitTypeDef gio;
  gio.Pin  = GPIO_PIN_4 | GPIO_PIN_5;
  gio.Mode = GPIO_MODE_ANALOG;
  gio.Pull = GPIO_NOPULL;
  HAL_GPIO_Init( GPIOA, &gio );
}

void HAL_DAC_MspDeInit( DAC_HandleTypeDef* dacHandle )
{
  if( dacHandle->Instance !=DAC ) {
    return;
  }

  __HAL_RCC_DAC_CLK_DISABLE();
  HAL_GPIO_DeInit( GPIOA, GPIO_PIN_4 | GPIO_PIN_5 );
}

