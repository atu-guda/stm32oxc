/**
  * @file    stm32f1xx_hal_conf.h
  * @brief   HAL configuration file. mod by atu.
  */

#ifndef __STM32F1xx_HAL_CONF_H
#define __STM32F1xx_HAL_CONF_H

// for oxc lib: just to check
#define REQ_MCBASE STM32F1
#define REQ_SYSCLK_FREQ 72

#define NEED_LEDS_EXTRA 1

#include <bsp/board_stm32f103c8_small.h>

#define STD_SYSTICK_HANDLER 1

// ########################## Module Selection ##############################
// base modules
#define USE_COMMON_HAL_MODULES

// AUX modules : full list: ../common/hal_modules_list.h
//

#include "../common/stm32f1xx_hal_conf_common.h"


#endif /* __STM32F1xx_HAL_CONF_H */
