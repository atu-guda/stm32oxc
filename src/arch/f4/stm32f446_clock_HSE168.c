#include <stm32f4xx_hal.h>
#include <errno.h>

#ifndef STM32F4
#error This SystemClock_Config is for stm32f4xx only
#endif

#if REQ_SYSCLK_FREQ != 168
#error This SystemClock_Config in for 168 MHz only
#endif

// 168 MHz  = 8 MHz, /8, *336, {/2,/7,/7},     /1,      /1,         /4,         /2
//            HSE     M     N    P  Q  R   AHB_PR  SYSTICK  APB1_PR=42  APB2_PR=84
// SDIO=48MHz, good for USB, 28 MHz ADC (/6)
// Scale1, -OverDrive, FLASH_LATENCY_5,
//

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
    .PLL.PLLN       = 336,
    .PLL.PLLP       = RCC_PLLP_DIV2,
    .PLL.PLLQ       = 7,
    .PLL.PLLR       = 7
  };

  if( HAL_RCC_OscConfig( (RCC_OscInitTypeDef *)(&RCC_OscInitStruct) ) != HAL_OK ) {
    errno = 1001;
    return  1001;
  }

  //
  // No overdrive
  //
  //

  static const RCC_ClkInitTypeDef RCC_ClkInitStruct = {
    .ClockType = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
               | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
    .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
    .AHBCLKDivider  = RCC_SYSCLK_DIV1,
    .APB1CLKDivider = RCC_HCLK_DIV4,
    .APB2CLKDivider = RCC_HCLK_DIV2
  };

  if( HAL_RCC_ClockConfig( (RCC_ClkInitTypeDef*)(&RCC_ClkInitStruct), FLASH_LATENCY_5 ) != HAL_OK ) {
    errno = 1003;
    return  1003;
  }

  static const RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {
    .PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_SDIO | RCC_PERIPHCLK_CLK48,
    .Clk48ClockSelection  = RCC_CLK48CLKSOURCE_PLLQ,
    .SdioClockSelection   = RCC_SDIOCLKSOURCE_CLK48,
  };

  if( HAL_RCCEx_PeriphCLKConfig( (RCC_PeriphCLKInitTypeDef*)(&PeriphClkInitStruct) ) != HAL_OK ) {
    errno = 1004;
    return  1004;
  }

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  approx_delay_calibrate();
  return 0;
}


