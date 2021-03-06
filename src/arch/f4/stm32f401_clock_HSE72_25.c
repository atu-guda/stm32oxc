#include <stm32f4xx_hal.h>
#include <errno.h>

#ifndef STM32F4
#error This SystemClock_Config is for stm32f4xx only
#endif

#if REQ_SYSCLK_FREQ != 72
#error This SystemClock_Config in for 72 MHz only
#endif

//  72 MHz  = 25 MHz, /25, *144, {/2,/3,/X},     /1,      /1,         /2,         /1
//            HSE     M       N    P  Q  -   AHB_PR  SYSTICK  APB1_PR=36  APB2_PR=72
// 36 MHz ADC (/2) (MAX)
// Scale2, -OverDrive, FLASH_LATENCY_2,
//

int SystemClockCfg(void); // copy from oxc_base.h to reduce deps
void  approx_delay_calibrate(void);


int SystemClockCfg(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG( PWR_REGULATOR_VOLTAGE_SCALE2 );

  static const RCC_OscInitTypeDef RCC_OscInitStruct = {
    .OscillatorType = RCC_OSCILLATORTYPE_HSE,
    .HSEState       = RCC_HSE_ON,
    .PLL.PLLState   = RCC_PLL_ON,
    .PLL.PLLSource  = RCC_PLLSOURCE_HSE,
    .PLL.PLLM       = 25,
    .PLL.PLLN       = 144,
    .PLL.PLLP       = RCC_PLLP_DIV2,
    .PLL.PLLQ       = 3
    // .PLL.PLLR    = 6
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
    .APB1CLKDivider = RCC_HCLK_DIV2,
    .APB2CLKDivider = RCC_HCLK_DIV1
  };

  if( HAL_RCC_ClockConfig( (RCC_ClkInitTypeDef*)(&RCC_ClkInitStruct), FLASH_LATENCY_2 ) != HAL_OK ) {
    errno = 1003;
    return  1003;
  }

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  approx_delay_calibrate();
  return 0;
}


