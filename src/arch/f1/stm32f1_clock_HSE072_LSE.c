#include <stm32f1xx_hal.h>
#include <errno.h>

#ifndef STM32F1
#error This SystemClock_Config is for stm32f1xx only
#endif

#if REQ_SYSCLK_FREQ != 72
#error This SystemClock_Config in for 72 MHz only
#endif

int SystemClockCfg(void); // copy from oxc_base.h to reduce deps
void  approx_delay_calibrate();

int SystemClockCfg(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE | RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState       = RCC_LSE_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL9;
  if( HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK ) {
    errno = 1001;
    return  1001;
  }

  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 ) != HAL_OK ) {
    errno = 1003;
    return  1003;
  }

  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection    = RCC_RTCCLKSOURCE_LSE;
  if( HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK ) {
    errno = 1004;
    return  1004;
  }

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  approx_delay_calibrate();
  return 0;
}


