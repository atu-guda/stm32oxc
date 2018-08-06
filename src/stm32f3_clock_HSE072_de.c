#include <stm32f3xx_hal.h>
#include <errno.h>

#ifndef STM32F3
#error This SystemClock_Config is for stm32f3xx only
#endif

#if REQ_SYSCLK_FREQ != 72
#error This SystemClock_Config in for 72 MHz only
#endif

//  72 MHz  = 8 MHz,               ,     /1,      /1,         /4,         /2
//            HSE                    -   AHB_PR  SYSTICK  APB1_PR=36  APB2_PR=72
// good for USB, 36 MHz ADC (MAX)
// FLASH_LATENCY_2,
// many devices is on

int SystemClockCfg(void); // copy from oxc_base.h to reduce deps
void  approx_delay_calibrate(void);

int SystemClockCfg(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV     = RCC_PREDIV_DIV1;
  if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK ) {
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
  PeriphClkInit.PeriphClockSelection =
      RCC_PERIPHCLK_USB    | RCC_PERIPHCLK_USART1
    | RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_UART4
    | RCC_PERIPHCLK_UART5  | RCC_PERIPHCLK_I2C1
    | RCC_PERIPHCLK_I2C2   | RCC_PERIPHCLK_TIM1
    | RCC_PERIPHCLK_TIM15  | RCC_PERIPHCLK_TIM16
    | RCC_PERIPHCLK_TIM17  | RCC_PERIPHCLK_TIM8
    | RCC_PERIPHCLK_ADC12  | RCC_PERIPHCLK_ADC34
    | RCC_PERIPHCLK_TIM2   | RCC_PERIPHCLK_TIM34;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;
  PeriphClkInit.Uart4ClockSelection  = RCC_UART4CLKSOURCE_SYSCLK;
  PeriphClkInit.Uart5ClockSelection  = RCC_UART5CLKSOURCE_SYSCLK;
  PeriphClkInit.Adc12ClockSelection  = RCC_ADC12PLLCLK_DIV1;
  PeriphClkInit.Adc34ClockSelection  = RCC_ADC34PLLCLK_DIV1;
  PeriphClkInit.I2c1ClockSelection   = RCC_I2C1CLKSOURCE_SYSCLK;
  PeriphClkInit.I2c2ClockSelection   = RCC_I2C2CLKSOURCE_SYSCLK;
  PeriphClkInit.USBClockSelection    = RCC_USBCLKSOURCE_PLL_DIV1_5;
  PeriphClkInit.Tim1ClockSelection   = RCC_TIM1CLK_HCLK;
  PeriphClkInit.Tim2ClockSelection   = RCC_TIM2CLK_HCLK;
  PeriphClkInit.Tim34ClockSelection  = RCC_TIM34CLK_HCLK;
  PeriphClkInit.Tim8ClockSelection   = RCC_TIM8CLK_HCLK;
  PeriphClkInit.Tim15ClockSelection  = RCC_TIM15CLK_HCLK;
  PeriphClkInit.Tim16ClockSelection  = RCC_TIM16CLK_HCLK;
  PeriphClkInit.Tim17ClockSelection  = RCC_TIM17CLK_HCLK;
  if( HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit ) != HAL_OK ) {
    errno = 1004;
    return  1004;
  }

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  HAL_SYSTICK_Config( HAL_RCC_GetHCLKFreq()/1000 ); // to HAL_delay work even before FreeRTOS start
  HAL_SYSTICK_CLKSourceConfig( SYSTICK_CLKSOURCE_HCLK );
  HAL_NVIC_SetPriority( SysTick_IRQn, OXC_SYSTICK_PRTY, 0 ); // will be readjusted by FreeRTOS
  approx_delay_calibrate();
  return 0;
}


