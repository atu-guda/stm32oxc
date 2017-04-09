#include <stm32f4xx_hal.h>
#include <errno.h>

#ifndef STM32F4
#error This SystemClock_Config is for stm32f4xx only
#endif

#if REQ_SYSCLK_FREQ != 180
#error This SystemClock_Config in for 180 MHz only
#endif

// 180 = /8, *360, /2
// 180 Mhz, no USB, 22.5 MHz for ADC (/8)

int SystemClockCfg(void); // copy from oxc_base.h to reduce deps

int SystemClockCfg(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  // RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 360;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  // RCC_OscInitStruct.PLL.PLLR = 6;
  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
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


  HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq()/1000 ); // to HAL_delay work even before FreeRTOS start
  HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );
  HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 ); // will be readjusted by FreeRTOS
  return 0;
}


