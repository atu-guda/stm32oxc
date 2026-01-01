#include <oxc_auto.h>

SD_HandleTypeDef hsd;

#ifdef SDIO
 #define SDIO_X SDIO
 #define SDIO_CLOCK_X   SDIO_CLOCK_EDGE_RISING
 #define SDIO_BYPASS_X  SDIO_CLOCK_POWER_SAVE_DISABLE
 #define SDIO_PS_X      SDIO_CLOCK_POWER_SAVE_DISABLE
 #define SDIO_BUS_X     SDIO_BUS_WIDE_1B
 #define SDIO_FLOW_X    SDIO_HARDWARE_FLOW_CONTROL_DISABLE
#else
 #define SDIO_X SDMMC1
 #define SDIO_CLOCK_X   SDMMC_CLOCK_EDGE_RISING
 #define SDIO_BYPASS_X  SDMMC_CLOCK_BYPASS_DISABLE
 #define SDIO_PS_X      SDMMC_CLOCK_POWER_SAVE_DISABLE
 #define SDIO_BUS_X     SDMMC_BUS_WIDE_1B
 #define SDIO_FLOW_X    SDMMC_HARDWARE_FLOW_CONTROL_DISABLE
#endif

void MX_SDIO_SD_Init()
{
  hsd.Instance                 = SDIO_X;
  hsd.Init.ClockEdge           = SDIO_CLOCK_X;
#if ! defined(STM32H7) && ! defined(STM32H5)
  hsd.Init.ClockBypass         = SDIO_BYPASS_X;
#endif
  hsd.Init.ClockPowerSave      = SDIO_PS_X;
  hsd.Init.BusWide             = SDIO_BUS_X;
  hsd.Init.HardwareFlowControl = SDIO_FLOW_X;
  hsd.Init.ClockDiv            = 8; // 200?
}

void HAL_SD_MspInit( SD_HandleTypeDef* /* sdHandle */ )
{
  // no check: the only SDIO
  SD_EXA_CLKEN;

  SD_EXA_CK. enableClk();
  SD_EXA_D0. enableClk();
  SD_EXA_CMD.enableClk();

  SD_EXA_CK. cfgAF( SD_EXA_GPIOAF );
  SD_EXA_D0. cfgAF( SD_EXA_GPIOAF );
  SD_EXA_CMD.cfgAF( SD_EXA_GPIOAF );
}

void HAL_SD_MspDeInit( SD_HandleTypeDef* /* sdHandle */ )
{
  SD_EXA_CLKDIS;
}

