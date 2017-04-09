#include <stm32f4xx_hal.h>
#include <errno.h>

#ifndef STM32F4
#error This SystemClock_Config is for stm32f4xx only
#endif

#if REQ_SYSCLK_FREQ != 168
#error This SystemClock_Config in for 168 MHz only
#endif

// 168 MHz  = 8 MHz, /4, *180, {/2,/7,/7},     /1,      /1,         /4,         /2
//            HSE     M     N    P  Q  R   AHB_PR  SYSTICK  APB1_PR=42  APB2_PR=84
// SDIO=48MHz, good for USB, 28 MHz ADC (/6)
// Scale1, -OverDrive, FLASH_LATENCY_5,
//

int SystemClockCfg(void); // copy from oxc_base.h to reduce deps

int SystemClockCfg(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 7;
  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )  {
    errno = 1001;
    return  1001;
  }

  //
  // No overdrive
  //

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                              | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  if( HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_5 ) != HAL_OK ) {
    errno = 1003;
    return  1003;
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_SDIO
                              | RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48CLKSOURCE_PLLQ;
  PeriphClkInitStruct.SdioClockSelection = RCC_SDIOCLKSOURCE_CLK48;
  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct ) != HAL_OK ) {
    errno = 1004;
    return  1004;
  }

  HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq()/1000 ); // to HAL_delay work even before FreeRTOS start
  HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );
  HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 ); // will be readjusted by FreeRTOS
  return 0;
}


