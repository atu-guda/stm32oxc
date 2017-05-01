#include <oxc_auto.h>

SD_HandleTypeDef hsd;

void MX_SDIO_SD_Init()
{
  hsd.Instance                 = SDIO;
  hsd.Init.ClockEdge           = SDIO_CLOCK_EDGE_RISING;
  hsd.Init.ClockBypass         = SDIO_CLOCK_BYPASS_DISABLE;
  hsd.Init.ClockPowerSave      = SDIO_CLOCK_POWER_SAVE_DISABLE;
  hsd.Init.BusWide             = SDIO_BUS_WIDE_1B;
  hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
  hsd.Init.ClockDiv            = 0; // 200?
}

void HAL_SD_MspInit( SD_HandleTypeDef* /* sdHandle */ )
{
  // no check: the only SDIO
  SD_EXA_CLKEN;

  GPIO_InitTypeDef gio;
  gio.Pin       = SD_EXA_CK_PIN;
  gio.Mode      = GPIO_MODE_AF_PP;
  gio.Pull      = GPIO_NOPULL;
  gio.Speed     = GPIO_SPEED_MAX;
  gio.Alternate = SD_EXA_GPIOAF;
  HAL_GPIO_Init( SD_EXA_CK_GPIO, &gio );

  gio.Pin       = SD_EXA_D0_PIN;
  HAL_GPIO_Init( SD_EXA_D0_GPIO, &gio );

  gio.Pin       = SD_EXA_CMD_PIN;
  HAL_GPIO_Init( SD_EXA_CMD_GPIO, &gio );
}

void HAL_SD_MspDeInit( SD_HandleTypeDef* /* sdHandle */ )
{
  SD_EXA_CLKDIS;
  HAL_GPIO_DeInit( SD_EXA_CK_GPIO,  SD_EXA_CK_PIN  );
  HAL_GPIO_DeInit( SD_EXA_D0_GPIO,  SD_EXA_D0_PIN  );
  HAL_GPIO_DeInit( SD_EXA_CMD_GPIO, SD_EXA_CMD_PIN );
}

