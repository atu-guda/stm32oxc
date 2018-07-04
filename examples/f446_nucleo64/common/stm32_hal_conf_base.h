#ifndef __STM32_HAL_CONF_H
#define __STM32_HAL_CONF_H

// base configuaration file for oxc f446_nucleo examples

#define REQ_MCBASE STM32F4
#ifndef REQ_SYSCLK_FREQ
  #define REQ_SYSCLK_FREQ 168
#endif

#define STD_SYSTICK_HANDLER 1

#define USE_COMMON_HAL_MODULES

#ifdef USE_COMMON_CONSOLE_MODULES
  #define HAL_UART_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_I2C
  #define HAL_I2C_MODULE_ENABLED
#endif

#ifdef USE_OXC_SPI
  #define HAL_SPI_MODULE_ENABLED
#endif

#ifdef USE_OXC_TIM
  #define HAL_TIM_MODULE_ENABLED
#endif

#ifdef USE_OXC_ADC
  #define HAL_ADC_MODULE_ENABLED
#endif

#ifdef USE_OXC_DAC
  #define HAL_DAC_MODULE_ENABLED
#endif

#ifdef USE_OXC_DMA
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_SD
  #define HAL_SD_MODULE_ENABLED
#endif

#include <bsp/board_stm32f446_nucleo64.h>

#include "stm32f4xx_hal_conf_common.h"


#endif /* __STM32_HAL_CONF_H */


