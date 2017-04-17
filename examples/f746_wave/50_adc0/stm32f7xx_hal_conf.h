/**
  * @file    stm32f7xx_hal_conf.h
  * @brief   HAL configuration file. mod by atu.
  */

#ifndef __STM32F7xx_HAL_CONF_H
#define __STM32F7xx_HAL_CONF_H

// for oxc lib: just to check
#define REQ_MCBASE STM32F7
#define REQ_SYSCLK_FREQ 200

#include <bsp/board_stm32f746_waveshare0.h>

#define STD_SYSTICK_HANDLER 1

// ########################## Module Selection ##############################
// base modules
#define USE_COMMON_HAL_MODULES

#define HAL_DMA_MODULE_ENABLED
#define HAL_UART_MODULE_ENABLED
#define HAL_ADC_MODULE_ENABLED

// AUX modules : full list: ../common/hal_modules_list.h
//

#include "../common/stm32f7xx_hal_conf_common.h"


#endif /* __STM32F7xx_HAL_CONF_H */


