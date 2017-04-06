#include <oxc_auto.h>

SD_HandleTypeDef hsd;

void MX_SDIO_SD_Init()
{
  hsd.Instance = SDIO;
  hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
  // hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_ENABLE;
  hsd.Init.ClockDiv = 0;
  // hsd.Init.ClockDiv = 200;
}

void HAL_SD_MspInit( SD_HandleTypeDef* /* sdHandle */ )
{
  // no check: the only SDIO
  __HAL_RCC_SDIO_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  // C8    ------> SDIO_D0
  // C12   ------> SDIO_CK
  // D2    ------> SDIO_CMD

  GPIO_InitTypeDef gis;
  gis.Pin = GPIO_PIN_8 | GPIO_PIN_12;
  gis.Mode = GPIO_MODE_AF_PP;
  gis.Pull = GPIO_NOPULL;
  gis.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  gis.Alternate = GPIO_AF12_SDIO;
  HAL_GPIO_Init( GPIOC, &gis );

  gis.Pin = GPIO_PIN_2;
  gis.Alternate = GPIO_AF12_SDIO;
  HAL_GPIO_Init( GPIOD, &gis );
}

void HAL_SD_MspDeInit( SD_HandleTypeDef* /* sdHandle */ )
{
    __HAL_RCC_SDIO_CLK_DISABLE();
    HAL_GPIO_DeInit( GPIOC, GPIO_PIN_8 | GPIO_PIN_12 );
    HAL_GPIO_DeInit( GPIOD, GPIO_PIN_2 );

}

