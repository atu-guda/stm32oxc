#ifndef __STM32_HAL_CONF_H
#define __STM32_HAL_CONF_H

// base configuaration file for oxc f746_nucleo examples

#define REQ_MCBASE STM32F7
#ifndef REQ_SYSCLK_FREQ
  #define REQ_SYSCLK_FREQ 200
#endif

#define STD_SYSTICK_HANDLER 1

#define USE_COMMON_HAL_MODULES

#ifdef USE_COMMON_CONSOLE_MODULES
  #define HAL_UART_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#include <bsp/board_stm32f746_nucleo.h>

#include "stm32f7xx_hal_conf_common.h"


#endif /* __STM32_HAL_CONF_H */


