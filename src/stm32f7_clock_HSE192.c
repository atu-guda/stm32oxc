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

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM       = 8;
  RCC_OscInitStruct.PLL.PLLN       = 384;
  RCC_OscInitStruct.PLL.PLLP       = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ       = 8;
  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )  {
    errno = 1001;
    return  1001;
  }

  if( HAL_PWREx_EnableOverDrive() != HAL_OK ) {
    errno = 1002;
    return  1002;
  }

  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_ClkInitStruct.ClockType =
      RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
    | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_6 ) != HAL_OK ) {
    errno = 1003;
    return  1003;
  }

  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
  PeriphClkInitStruct.PeriphClockSelection =
      RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_USART2
    | RCC_PERIPHCLK_I2C1   | RCC_PERIPHCLK_I2C2
    | RCC_PERIPHCLK_SDMMC1 | RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN       = 192;
  PeriphClkInitStruct.PLLSAI.PLLSAIR       = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ       = 2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP       = RCC_PLLSAIP_DIV4;
  PeriphClkInitStruct.PLLSAIDivQ           = 1;
  PeriphClkInitStruct.PLLSAIDivR           = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  PeriphClkInitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInitStruct.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_SYSCLK;
  PeriphClkInitStruct.I2c2ClockSelection   = RCC_I2C2CLKSOURCE_SYSCLK;
  PeriphClkInitStruct.Clk48ClockSelection  = RCC_CLK48SOURCE_PLLSAIP;
  PeriphClkInitStruct.Sdmmc1ClockSelection = RCC_SDMMC1CLKSOURCE_CLK48;
  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct ) != HAL_OK )  {
    errno = 1007;
    return  1007;
  }

  SCB_EnableICache(); /* Enable I-Cache */
  SCB_EnableDCache(); /* Enable D-Cache */

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq()/1000 ); // to HAL_delay work even before FreeRTOS start
  HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );
  HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 ); // will be readjusted by FreeRTOS
  approx_delay_calibrate();
  return 0;
}


