#include <oxc_base.h>

#ifndef STM32F7
#error This SystemClock_Config is for stm32f7xx only
#endif

#if REQ_SYSCLK_FREQ != 200
#error This SystemClock_Config in for 200 MHz only
#endif


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;

  /**Configure the main internal regulator output voltage  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE1 );

  /**Initializes the CPU, AHB and APB busses clocks */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )  {
    Error_Handler( 13 );
  }

  if( HAL_PWREx_EnableOverDrive() != HAL_OK )  { /**Activate the Over-Drive mode  */
    Error_Handler( 14 );
  }

  /**Initializes the CPU, AHB and APB busses clocks  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
    | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if( HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK ) {
    Error_Handler( 15 );
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART1 | RCC_PERIPHCLK_I2C1
    | RCC_PERIPHCLK_CLK48;
  PeriphClkInitStruct.PLLSAI.PLLSAIN = 96;
  PeriphClkInitStruct.PLLSAI.PLLSAIR =  2;
  PeriphClkInitStruct.PLLSAI.PLLSAIQ =  2;
  PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
  PeriphClkInitStruct.PLLSAIDivQ = 1;
  PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_2;
  PeriphClkInitStruct.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  PeriphClkInitStruct.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
  if( HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK )  {
    Error_Handler( 15 );
  }

  /**Configure the Systick interrupt time   */
  HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq()/1000 );

  /**Configure the Systick   */
  HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );

  /* SysTick_IRQn interrupt configuration */
  // HAL_NVIC_SetPriority( SysTick_IRQn, 0, 0 );
}


