#include <oxc_gpio.h>

#include <board_sdram.h>

// FMC GPIO Configuration
//   PF0   ------> FMC_A0
//   PF1   ------> FMC_A1
//   PF2   ------> FMC_A2
//   PF3   ------> FMC_A3
//   PF4   ------> FMC_A4
//   PF5   ------> FMC_A5
//   PH5   ------> FMC_SDNWE
//   PF11  ------> FMC_SDNRAS
//   PF12  ------> FMC_A6
//   PF13  ------> FMC_A7
//   PF14  ------> FMC_A8
//   PF15  ------> FMC_A9
//   PG0   ------> FMC_A10
//   PG1   ------> FMC_A11
//   PG2   ------> FMC_A12
//   PE7   ------> FMC_D4
//   PE8   ------> FMC_D5
//   PE9   ------> FMC_D6
//   PE10  ------> FMC_D7
//   PE11  ------> FMC_D8
//   PE12  ------> FMC_D9
//   PE13  ------> FMC_D10
//   PE14  ------> FMC_D11
//   PE15  ------> FMC_D12
//
//   PH3   ------> FMC_SDNE0 ??
//   PH2   ------> FMC_SDCKE0 ??
//
//   PD8   ------> FMC_D13
//   PD9   ------> FMC_D14
//   PD10  ------> FMC_D15
//   PD14  ------> FMC_D0
//   PD15  ------> FMC_D1
//   PG4   ------> FMC_BA0
//   PG5   ------> FMC_BA1
//   PG8   ------> FMC_SDCLK
//   PD0   ------> FMC_D2
//   PD1   ------> FMC_D3
//   PG15  ------> FMC_SDNCAS
//   PE0   ------> FMC_NBL0
//   PE1   ------> FMC_NBL1

void HAL_SDRAM_MspInit( SDRAM_HandleTypeDef * /* hsdram */ )
{
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_FMC_CLK_ENABLE();

  GpioF.cfgAF_N( PinMask(
        GPIO_PIN_0  | GPIO_PIN_1  | GPIO_PIN_2  | GPIO_PIN_3
      | GPIO_PIN_4  | GPIO_PIN_5  | GPIO_PIN_11 | GPIO_PIN_12
      | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 ), GPIO_AF12_FMC   );

  GpioH.cfgAF_N( PinMask( GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_5 ), GPIO_AF12_FMC );

  GpioG.cfgAF_N( PinMask(
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 |  GPIO_PIN_4 | GPIO_PIN_5
      | GPIO_PIN_8 | GPIO_PIN_15 ),   GPIO_AF12_FMC );

  GpioE.cfgAF_N( PinMask(
        GPIO_PIN_7  | GPIO_PIN_8  | GPIO_PIN_9  | GPIO_PIN_10
      | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14
      | GPIO_PIN_15 | GPIO_PIN_0  | GPIO_PIN_1 ),  GPIO_AF12_FMC );

  GpioD.cfgAF_N( PinMask(
        GPIO_PIN_8  | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14
      | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1 ),  GPIO_AF12_FMC );
}


