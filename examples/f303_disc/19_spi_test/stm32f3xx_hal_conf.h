/**
  * @file    stm32f3xx_hal_conf.h
  * @brief   HAL configuration file. mod by atu.
  */

#ifndef __STM32F3xx_HAL_CONF_H
#define __STM32F3xx_HAL_CONF_H

// for oxc lib: just to check
#define REQ_MCBASE STM32F3
#define REQ_SYSCLK_FREQ 72

#include <bsp/board_stm32f3discovery.h>

#define STD_SYSTICK_HANDLER 1

// ########################## Module Selection ##############################
// base modules
#define USE_COMMON_HAL_MODULES

// AUX modules : full list: ../common/hal_modules_list.h
//
#define HAL_UART_MODULE_ENABLED
#define HAL_DMA_MODULE_ENABLED
#define HAL_SPI_MODULE_ENABLED

#include "../common/stm32f3xx_hal_conf_common.h"

#define USE_SPI_CRC 0

#endif /* __STM32F3xx_HAL_CONF_H */
