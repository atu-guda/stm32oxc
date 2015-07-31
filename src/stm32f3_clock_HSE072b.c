#include <oxc_base.h>
// nax 72MHz clock for stm32f3discovery with external HSE 8MHz source

#ifndef STM32F3
#error This SystemClock_Config is for stm32f3xx only
#endif

#if REQ_SYSCLK_FREQ != 72
#error This SystemClock_Config in for 72 MHz only
#endif


void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_PeriphCLKInitTypeDef PeriphClkInit;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  // RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  HAL_RCC_OscConfig( &RCC_OscInitStruct );

  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 );

  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
                              |RCC_PERIPHCLK_TIM1|RCC_PERIPHCLK_TIM8
                              |RCC_PERIPHCLK_ADC12;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
  PeriphClkInit.USBClockSelection = RCC_USBPLLCLK_DIV1_5;
  PeriphClkInit.Tim1ClockSelection = RCC_TIM1CLK_PLLCLK;
  PeriphClkInit.Tim8ClockSelection = RCC_TIM8CLK_PLLCLK;
  HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit );

  // PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB|RCC_PERIPHCLK_USART1
  //                             |RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_I2C1
  //                             |RCC_PERIPHCLK_TIM8|RCC_PERIPHCLK_ADC12;
  // PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK2;
  // PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  // PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
  // PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_SYSCLK;
  // PeriphClkInit.USBClockSelection = RCC_USBPLLCLK_DIV1_5;
  // PeriphClkInit.Tim8ClockSelection = RCC_TIM8CLK_HCLK;
  // HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit );

  __SYSCFG_CLK_ENABLE();
}


