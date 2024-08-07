#include <stm32g4xx_hal.h>
#include <errno.h>

#ifndef STM32G4
#error This SystemClock_Config is for stm32g4xx only
#endif

#if REQ_SYSCLK_FREQ != 144
#error This SystemClock_Config in for 144 MHz only
#endif

// 144 MHz  = 8 MHz, /1,  *36, {/2,/6,/2},     /1,      /1,         /1,         /1
//            HSE     M     N    P  Q  R   AHB_PR  SYSTICK  APB1_PR=144  APB2_PR=144
// Scale1, -OverDrive, FLASH_LATENCY_4,
//

int SystemClockCfg(void); // copy from oxc_base.h to reduce deps
void  approx_delay_calibrate(void);

int SystemClockCfg(void)
{
  __HAL_RCC_PWR_CLK_ENABLE();
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);

  static const RCC_OscInitTypeDef RCC_OscInitStruct = {
    .OscillatorType = RCC_OSCILLATORTYPE_HSE,
    .HSEState       = RCC_HSE_ON,
    .PLL.PLLState   = RCC_PLL_ON,
    .PLL.PLLSource  = RCC_PLLSOURCE_HSE,
    .PLL.PLLM       = RCC_PLLM_DIV1,
    .PLL.PLLN       = 36,
    .PLL.PLLP       = RCC_PLLP_DIV2,
    .PLL.PLLQ       = RCC_PLLQ_DIV6,
    .PLL.PLLR       = RCC_PLLR_DIV2,
  };

  if( HAL_RCC_OscConfig( (RCC_OscInitTypeDef *)(&RCC_OscInitStruct) ) != HAL_OK ) {
    errno = 1001;
    return  1001;
  }


  static const RCC_ClkInitTypeDef RCC_ClkInitStruct = {
    .ClockType = RCC_CLOCKTYPE_HCLK  | RCC_CLOCKTYPE_SYSCLK
               | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2,
    .SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK,
    .AHBCLKDivider  = RCC_SYSCLK_DIV1,
    .APB1CLKDivider = RCC_HCLK_DIV1,
    .APB2CLKDivider = RCC_HCLK_DIV1,
  };

  if( HAL_RCC_ClockConfig( (RCC_ClkInitTypeDef*)(&RCC_ClkInitStruct), FLASH_LATENCY_4 ) != HAL_OK ) {
    errno = 1003;
    return  1003;
  }

  // static const RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {
  //  .PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_SDIO | RCC_PERIPHCLK_CLK48,
  //  .Clk48ClockSelection  = RCC_CLK48CLKSOURCE_PLLQ,
  //  .SdioClockSelection   = RCC_SDIOCLKSOURCE_CLK48,
  //};

  //if( HAL_RCCEx_PeriphCLKConfig( (RCC_PeriphCLKInitTypeDef*)(&PeriphClkInitStruct) ) != HAL_OK ) {
  //  errno = 1004;
  //  return  1004;
  //}

  __HAL_RCC_SYSCFG_CLK_ENABLE();

  approx_delay_calibrate();
  return 0;
}


