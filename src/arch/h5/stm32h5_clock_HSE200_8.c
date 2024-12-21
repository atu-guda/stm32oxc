#include <stm32h5xx_hal.h>
#include <stm32h5xx_hal_icache.h>
#include <errno.h>

#ifndef STM32H5
#error This SystemClockCfg is for stm32h5xx only
#endif

#if REQ_SYSCLK_FREQ != 200
#error This SystemClockCfg in for 200 MHz only
#endif


int SystemClockCfg(void); // copy from oxc_base.h to reduce deps
void  approx_delay_calibrate(void);

int SystemClockCfg(void)
{
  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

  while( !__HAL_PWR_GET_FLAG( PWR_FLAG_VOSRDY ) ) {}

  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG( RCC_LSEDRIVE_LOW );

  static const RCC_OscInitTypeDef RCC_OscInitStruct = {
    .OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE,
    .HSEState       = RCC_HSE_ON,
    .LSEState       = RCC_LSE_ON,
    .PLL.PLLState   = RCC_PLL_ON,
    .PLL.PLLSource  = RCC_PLL1_SOURCE_HSE,
    .PLL.PLLM       = 8,
    .PLL.PLLN       = 400,
    .PLL.PLLP       = 2,
    .PLL.PLLQ       = 2,
    .PLL.PLLR       = 2,
    .PLL.PLLRGE     = RCC_PLL1_VCIRANGE_0,
    .PLL.PLLVCOSEL  = RCC_PLL1_VCORANGE_WIDE,
    .PLL.PLLFRACN   = 0
  };
  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
    errno = 1000;
    return 0;
  }

  static const RCC_ClkInitTypeDef RCC_ClkInitStruct = {
    .ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
               | RCC_CLOCKTYPE_PCLK1| RCC_CLOCKTYPE_PCLK2
               | RCC_CLOCKTYPE_PCLK3,
    .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
    .AHBCLKDivider  = RCC_SYSCLK_DIV1,
    .APB1CLKDivider = RCC_HCLK_DIV1,
    .APB2CLKDivider = RCC_HCLK_DIV1,
    .APB3CLKDivider = RCC_HCLK_DIV1
  };
  if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_5 ) != HAL_OK ) {
    errno = 1001;
    return 0;
  }

  __HAL_FLASH_SET_PROGRAM_DELAY( FLASH_PROGRAMMING_DELAY_2 );

  static const RCC_PeriphCLKInitTypeDef PeriphClkInitStruct  = {
    .PeriphClockSelection = RCC_PERIPHCLK_ADCDAC | RCC_PERIPHCLK_SPI2,
    .PLL2.PLL2Source      = RCC_PLL2_SOURCE_HSE,
    .PLL2.PLL2M           = 8,
    .PLL2.PLL2N           = 400,
    .PLL2.PLL2P           = 2,
    .PLL2.PLL2Q           = 2,
    .PLL2.PLL2R           = 4,
    .PLL2.PLL2RGE         = RCC_PLL2_VCIRANGE_0,
    .PLL2.PLL2VCOSEL      = RCC_PLL2_VCORANGE_MEDIUM,
    .PLL2.PLL2FRACN       = 0,
    .PLL2.PLL2ClockOut    = RCC_PLL2_DIVP | RCC_PLL2_DIVR,
    .AdcDacClockSelection = RCC_ADCDACCLKSOURCE_PLL2R,
    .Spi2ClockSelection   = RCC_SPI2CLKSOURCE_PLL2P
  };
  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct ) != HAL_OK ) {
    errno = 1002;
    return 0;
  }

  if( HAL_ICACHE_ConfigAssociativityMode( ICACHE_1WAY ) != HAL_OK ) {
    errno = 1003;
    return 0;
  }
  if( HAL_ICACHE_Enable() != HAL_OK ) {
    errno = 1004;
    return 0;
  }

  return 1;
}

