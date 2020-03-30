#include <stm32f3xx_hal.h>
#include <errno.h>

#ifndef STM32F334
#error This SystemClock_Config is for stm32f334 only
#endif

#if REQ_SYSCLK_FREQ != 72
#error This SystemClock_Config in for 72 MHz only
#endif

int SystemClockCfg(void); // copy from oxc_base.h to reduce deps
void  approx_delay_calibrate(void);

int SystemClockCfg(void)
{
  static const RCC_OscInitTypeDef RCC_OscInitStruct = {
    .OscillatorType      = RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE,
    .HSEState            = RCC_HSE_BYPASS,
    .HSEPredivValue      = RCC_HSE_PREDIV_DIV1,
    .HSIState            = RCC_HSI_ON,
    .HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT,
    .PLL.PLLState        = RCC_PLL_ON,
    .PLL.PLLSource       = RCC_PLLSOURCE_HSE,
    .PLL.PLLMUL          = RCC_PLL_MUL9
  };

  if( HAL_RCC_OscConfig( (RCC_OscInitTypeDef*)( &RCC_OscInitStruct ) ) != HAL_OK ) {
    errno = 1001;
    return  1001;
  }

  static const RCC_ClkInitTypeDef RCC_ClkInitStruct = {
    .ClockType =
        RCC_CLOCKTYPE_SYSCLK
      | RCC_CLOCKTYPE_HCLK
      | RCC_CLOCKTYPE_PCLK1
      | RCC_CLOCKTYPE_PCLK2,
    .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
    .AHBCLKDivider  = RCC_SYSCLK_DIV1,
    .APB1CLKDivider = RCC_HCLK_DIV2,
    .APB2CLKDivider = RCC_HCLK_DIV1
  };

  if( HAL_RCC_ClockConfig( (RCC_ClkInitTypeDef*)( &RCC_ClkInitStruct ), FLASH_LATENCY_2 ) != HAL_OK ) {
    errno = 1003;
    return  1003;
  }

  static const RCC_PeriphCLKInitTypeDef PeriphClkInit = {
    .PeriphClockSelection =
        RCC_PERIPHCLK_HRTIM1
      | RCC_PERIPHCLK_USART1
      | RCC_PERIPHCLK_I2C1
      | RCC_PERIPHCLK_TIM1
      | RCC_PERIPHCLK_ADC12,
    .Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK,
    .Adc12ClockSelection  = RCC_ADC12PLLCLK_DIV1,
    .I2c1ClockSelection   = RCC_I2C1CLKSOURCE_SYSCLK,
    .Tim1ClockSelection   = RCC_TIM1CLK_PLLCLK,
    .Hrtim1ClockSelection = RCC_HRTIM1CLK_PLLCLK
  };

  if( HAL_RCCEx_PeriphCLKConfig( (RCC_PeriphCLKInitTypeDef*)( &PeriphClkInit ) ) != HAL_OK ) {
    errno = 1004;
    return  1004;
  }

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  approx_delay_calibrate();
  return 0;
}


