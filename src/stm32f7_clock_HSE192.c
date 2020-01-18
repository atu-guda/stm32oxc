#include <stm32f7xx_hal.h>
#include <errno.h>

#ifndef STM32F7
#error This SystemClock_Config is for stm32f7xx only
#endif

#if REQ_SYSCLK_FREQ != 192
#error This SystemClock_Config in for 192 MHz only
#endif


int SystemClockCfg(void); // copy from oxc_base.h to reduce deps
void  approx_delay_calibrate(void);

int SystemClockCfg(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

  static const RCC_OscInitTypeDef RCC_OscInitStruct = {
    .OscillatorType = RCC_OSCILLATORTYPE_HSE,
    .HSEState       = RCC_HSE_ON,
    .PLL.PLLState   = RCC_PLL_ON,
    .PLL.PLLSource  = RCC_PLLSOURCE_HSE,
    .PLL.PLLM       = 8,
    .PLL.PLLN       = 384,
    .PLL.PLLP       = RCC_PLLP_DIV2,
    .PLL.PLLQ       = 8,
    // .PLL.PLLR    = 8
  };

  if( HAL_RCC_OscConfig( (RCC_OscInitTypeDef *)(&RCC_OscInitStruct) ) != HAL_OK ) {
    errno = 1001;
    return  1001;
  }

  if( HAL_PWREx_EnableOverDrive() != HAL_OK ) {
    errno = 1002;
    return  1002;
  }

  static const RCC_ClkInitTypeDef RCC_ClkInitStruct = {
    .ClockType = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
               | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
    .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
    .AHBCLKDivider  = RCC_SYSCLK_DIV1,
    .APB1CLKDivider = RCC_HCLK_DIV4,
    .APB2CLKDivider = RCC_HCLK_DIV2
  };

  if( HAL_RCC_ClockConfig( (RCC_ClkInitTypeDef*)(&RCC_ClkInitStruct), FLASH_LATENCY_6 ) != HAL_OK ) {
    errno = 1003;
    return  1003;
  }

  static const RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {
    .PeriphClockSelection =
        RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2
      | RCC_PERIPHCLK_I2C1   | RCC_PERIPHCLK_I2C2
      | RCC_PERIPHCLK_SDMMC1 | RCC_PERIPHCLK_CLK48,
    .PLLSAI.PLLSAIN       = 192,
    .PLLSAI.PLLSAIR       = 2,
    .PLLSAI.PLLSAIQ       = 2,
    .PLLSAI.PLLSAIP       = RCC_PLLSAIP_DIV4,
    .PLLSAIDivQ           = 1,
    .PLLSAIDivR           = RCC_PLLSAIDIVR_2,
    .Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK,
    .Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK,
    .I2c1ClockSelection   = RCC_I2C1CLKSOURCE_SYSCLK,
    .I2c2ClockSelection   = RCC_I2C2CLKSOURCE_SYSCLK,
    .Clk48ClockSelection  = RCC_CLK48SOURCE_PLLSAIP,
    .Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48
  };

  if( HAL_RCCEx_PeriphCLKConfig( (RCC_PeriphCLKInitTypeDef*)(&PeriphClkInitStruct) ) != HAL_OK )  {
    errno = 1007;
    return  1007;
  }

  SCB_EnableICache(); /* Enable I-Cache */
  SCB_EnableDCache(); /* Enable D-Cache */

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  approx_delay_calibrate();
  return 0;
}


