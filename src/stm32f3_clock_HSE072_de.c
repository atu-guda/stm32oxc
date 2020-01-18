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
  static const RCC_OscInitTypeDef RCC_OscInitStruct = {
    .OscillatorType = RCC_OSCILLATORTYPE_HSE,
    .HSEState       = RCC_HSE_ON,
    .PLL.PREDIV     = RCC_PREDIV_DIV1,
    .PLL.PLLState   = RCC_PLL_ON,
    .PLL.PLLSource  = RCC_PLLSOURCE_HSE,
    .PLL.PLLMUL     = RCC_PLL_MUL9
  };

  if( HAL_RCC_OscConfig( (RCC_OscInitTypeDef*)(&RCC_OscInitStruct) ) != HAL_OK ) {
    errno = 1001;
    return  1001;
  }

  static const RCC_ClkInitTypeDef RCC_ClkInitStruct = {
    .ClockType      = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
                    | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
    .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
    .AHBCLKDivider  = RCC_SYSCLK_DIV1,
    .APB1CLKDivider = RCC_HCLK_DIV2,
    .APB2CLKDivider = RCC_HCLK_DIV1,
  };

  if( HAL_RCC_ClockConfig( (RCC_ClkInitTypeDef *)(&RCC_ClkInitStruct), FLASH_LATENCY_2 ) != HAL_OK ) {
    errno = 1003;
    return  1003;
  }

  static const RCC_PeriphCLKInitTypeDef PeriphClkInit = {
    .PeriphClockSelection =
      RCC_PERIPHCLK_USB    | RCC_PERIPHCLK_USART1
      | RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_UART4
      | RCC_PERIPHCLK_UART5  | RCC_PERIPHCLK_I2C1
      | RCC_PERIPHCLK_I2C2   | RCC_PERIPHCLK_TIM1
      | RCC_PERIPHCLK_TIM15  | RCC_PERIPHCLK_TIM16
      | RCC_PERIPHCLK_TIM17  | RCC_PERIPHCLK_TIM8
      | RCC_PERIPHCLK_ADC12  | RCC_PERIPHCLK_ADC34
      | RCC_PERIPHCLK_TIM2   | RCC_PERIPHCLK_TIM34,
    .Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK,
    .Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK,
    .Uart4ClockSelection  = RCC_UART4CLKSOURCE_SYSCLK,
    .Uart5ClockSelection  = RCC_UART5CLKSOURCE_SYSCLK,
    .Adc12ClockSelection  = RCC_ADC12PLLCLK_DIV1,
    .Adc34ClockSelection  = RCC_ADC34PLLCLK_DIV1,
    .I2c1ClockSelection   = RCC_I2C1CLKSOURCE_SYSCLK,
    .I2c2ClockSelection   = RCC_I2C2CLKSOURCE_SYSCLK,
    .USBClockSelection    = RCC_USBCLKSOURCE_PLL_DIV1_5,
    .Tim1ClockSelection   = RCC_TIM1CLK_HCLK,
    .Tim2ClockSelection   = RCC_TIM2CLK_HCLK,
    .Tim34ClockSelection  = RCC_TIM34CLK_HCLK,
    .Tim8ClockSelection   = RCC_TIM8CLK_HCLK,
    .Tim15ClockSelection  = RCC_TIM15CLK_HCLK,
    .Tim16ClockSelection  = RCC_TIM16CLK_HCLK,
    .Tim17ClockSelection  = RCC_TIM17CLK_HCLK
  };

  if( HAL_RCCEx_PeriphCLKConfig( (RCC_PeriphCLKInitTypeDef*)(&PeriphClkInit) ) != HAL_OK ) {
    errno = 1004;
    return  1004;
  }

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  approx_delay_calibrate();
  return 0;
}


