/**
  * @file    stm32f4xx_hal_conf.h
  * @brief   HAL configuration file. mod by atu.
  */

#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

// for oxc lib: just to check
#define REQ_MCBASE STM32F4
#define REQ_SYSCLK_FREQ 168

#include <bsp/board_stm32f407_atu_x2.h>

// now even for FreeRTOS
#define STD_SYSTICK_HANDLER 1

// ########################## Module Selection ##############################
// base modules
#define USE_COMMON_HAL_MODULES

// AUX modules : full list: ../common/hal_modules_list.h
//
#define HAL_DMA_MODULE_ENABLED
#define HAL_PCD_MODULE_ENABLED
#define HAL_DAC_MODULE_ENABLED

#include "../common/stm32f4xx_hal_conf_common.h"


#endif /* __STM32F4xx_HAL_CONF_H */


