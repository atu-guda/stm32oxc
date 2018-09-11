#include <errno.h>
#include <oxc_auto.h>

#include "meas0.h"

ADC_HandleTypeDef hadc1;
DMA_HandleTypeDef hdma_adc1;

int MX_ADC1_Init()
{
  hadc1.Instance                   = ADC1;
  hadc1.Init.ClockPrescaler        = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode          = ENABLE;
  hadc1.Init.ContinuousConvMode    = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv      = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion       = n_adc;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection          = ADC_EOC_SEQ_CONV;
  if( HAL_ADC_Init( &hadc1 ) != HAL_OK ) {
    return 0;
  }

  ADC_ChannelConfTypeDef sConfig;
  sConfig.Channel      = ADC_CHANNEL_10;
  sConfig.Rank         = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK ) {
    return 0;
  }

  sConfig.Channel = ADC_CHANNEL_11;
  sConfig.Rank    = 2;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK ) {
    return 0;
  }

  sConfig.Channel = ADC_CHANNEL_12;
  sConfig.Rank    = 3;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK )  {
    return 0;
  }

  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = 4;
  if( HAL_ADC_ConfigChannel( &hadc1, &sConfig ) != HAL_OK) {
    return 0;
  }
  return 1;
}

void HAL_ADC_MspInit( ADC_HandleTypeDef* adcHandle )
{
  GPIO_InitTypeDef gio;
  if( adcHandle->Instance == ADC1 ) {
    __HAL_RCC_ADC1_CLK_ENABLE();
    /** PC0 --> ADC1_IN10 PC1 --> ADC1_IN11 PC2 --> ADC1_IN12 PC3 --> ADC1_IN13 */
    gio.Pin  = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
    gio.Mode = GPIO_MODE_ANALOG;
    gio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init( GPIOC, &gio );

    if( ! dma_subinit() ) {
      return;
    }

    // __HAL_LINKDMA( adcHandle, DMA_Handle, hdma_adc1 );
  }
}

int  dma_subinit()
{
  hdma_adc1.Instance                 = DMA2_Stream0;
  hdma_adc1.Init.Channel             = DMA_CHANNEL_0;
  hdma_adc1.Init.Direction           = DMA_PERIPH_TO_MEMORY;
  hdma_adc1.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_adc1.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_adc1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
  hdma_adc1.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
  hdma_adc1.Init.Mode                = DMA_NORMAL;
  hdma_adc1.Init.Priority            = DMA_PRIORITY_MEDIUM;
  hdma_adc1.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  if( HAL_DMA_Init( &hdma_adc1 ) != HAL_OK ) {
    errno |= 0x1000000;
    return 0;
  }
  __HAL_LINKDMA( &hadc1, DMA_Handle, hdma_adc1 );
  return 1;
}

void HAL_ADC_MspDeInit( ADC_HandleTypeDef* adcHandle )
{
  if( adcHandle->Instance == ADC1 )  {
    __HAL_RCC_ADC1_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3 );
    HAL_DMA_DeInit( adcHandle->DMA_Handle );
  }
}

int MX_DMA_Init()
{
  __HAL_RCC_DMA2_CLK_ENABLE();
  HAL_NVIC_SetPriority( DMA2_Stream0_IRQn, 2, 0 );
  HAL_NVIC_EnableIRQ( DMA2_Stream0_IRQn );
  return 1;
}

