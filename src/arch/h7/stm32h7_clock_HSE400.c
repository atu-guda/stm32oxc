#include <stm32h7xx_hal.h>
#include <errno.h>

#ifndef STM32H7
#error This SystemClock_Config is for stm32h7xx only
#endif

#if REQ_SYSCLK_FREQ != 400
#error This SystemClock_Config in for 400 MHz only
#endif


int SystemClockCfg(void); // copy from oxc_base.h to reduce deps
void  approx_delay_calibrate(void);

int SystemClockCfg(void)
{
  HAL_PWREx_ConfigSupply( PWR_LDO_SUPPLY );
  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  while( ! __HAL_PWR_GET_FLAG( PWR_FLAG_VOSRDY ) ) { /* NOP*/ }

  static const RCC_OscInitTypeDef RCC_OscInitStruct = {
    .OscillatorType = RCC_OSCILLATORTYPE_HSE,
    .HSEState       = RCC_HSE_ON,
    .PLL.PLLState   = RCC_PLL_ON,
    .PLL.PLLSource  = RCC_PLLSOURCE_HSE,
    .PLL.PLLM       = 4,
    .PLL.PLLN       = 400,
    .PLL.PLLP       = 2,
    .PLL.PLLQ       = 2,
    .PLL.PLLR       = 2,
    .PLL.PLLRGE     = RCC_PLL1VCIRANGE_1,
    .PLL.PLLVCOSEL  = RCC_PLL1VCOWIDE,
    .PLL.PLLFRACN   = 0
  };
  if( HAL_RCC_OscConfig( (RCC_OscInitTypeDef*)(&RCC_OscInitStruct) ) != HAL_OK ) {
    errno = 1001;
    return 1001;
  }

  static const RCC_ClkInitTypeDef RCC_ClkInitStruct = {
    .ClockType = RCC_CLOCKTYPE_HCLK    | RCC_CLOCKTYPE_SYSCLK
               | RCC_CLOCKTYPE_PCLK1   | RCC_CLOCKTYPE_PCLK2
               | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1,
    .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
    .SYSCLKDivider  = RCC_SYSCLK_DIV1,
    .AHBCLKDivider  = RCC_HCLK_DIV2,
    .APB3CLKDivider = RCC_APB3_DIV2,
    .APB1CLKDivider = RCC_APB1_DIV2,
    .APB2CLKDivider = RCC_APB2_DIV2,
    .APB4CLKDivider = RCC_APB4_DIV2
  };

  if( HAL_RCC_ClockConfig( (RCC_ClkInitTypeDef*)(&RCC_ClkInitStruct), FLASH_LATENCY_2 ) != HAL_OK ) {
    errno = 1003;
    return 1003;
  }

  static const RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {
    .PeriphClockSelection =
        RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_RNG  | RCC_PERIPHCLK_SPI2
      | RCC_PERIPHCLK_SDMMC  | RCC_PERIPHCLK_ADC
      | RCC_PERIPHCLK_I2C1   | RCC_PERIPHCLK_I2C4 | RCC_PERIPHCLK_USB
      | RCC_PERIPHCLK_FMC,
    .PLL2.PLL2M            = 4,
    .PLL2.PLL2N            = 96,
    .PLL2.PLL2P            = 2,
    .PLL2.PLL2Q            = 4,
    .PLL2.PLL2R            = 2,
    .PLL2.PLL2RGE          = RCC_PLL2VCIRANGE_1,
    .PLL2.PLL2VCOSEL       = RCC_PLL2VCOWIDE,
    .PLL2.PLL2FRACN        = 0,
    .PLL3.PLL3M            = 4,
    .PLL3.PLL3N            = 96,
    .PLL3.PLL3P            = 2,
    .PLL3.PLL3Q            = 4,
    .PLL3.PLL3R            = 2,
    .PLL3.PLL3RGE          = RCC_PLL3VCIRANGE_1,
    .PLL3.PLL3VCOSEL       = RCC_PLL3VCOWIDE,
    .PLL3.PLL3FRACN        = 0,
    .FmcClockSelection     = RCC_FMCCLKSOURCE_D1HCLK,
    .SdmmcClockSelection   = RCC_SDMMCCLKSOURCE_PLL,
    .Spi123ClockSelection  = RCC_SPI123CLKSOURCE_PLL2,
    .Usart16ClockSelection = RCC_USART16CLKSOURCE_D2PCLK2,
    .I2c123ClockSelection  = RCC_I2C123CLKSOURCE_D2PCLK1,
    .UsbClockSelection     = RCC_USBCLKSOURCE_PLL3,
    .AdcClockSelection     = RCC_ADCCLKSOURCE_PLL2
  };
  if( HAL_RCCEx_PeriphCLKConfig( (RCC_PeriphCLKInitTypeDef*)(&PeriphClkInitStruct) ) != HAL_OK ) {
    errno = 1007;
    return 1007;
  }

  HAL_PWREx_EnableUSBVoltageDetector();

  approx_delay_calibrate();

  return 0;
}


