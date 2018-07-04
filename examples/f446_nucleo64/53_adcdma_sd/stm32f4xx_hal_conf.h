#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

// for fastest ADC 36MHz clocking
#define REQ_SYSCLK_FREQ 144

// ########################## Module Selection ##############################
// base modules
#define USE_COMMON_HAL_MODULES
#define USE_COMMON_CONSOLE_MODULES

// AUX modules : full list: ../common/hal_modules_list.h
//
#define HAL_ADC_MODULE_ENABLED
#define HAL_TIM_MODULE_ENABLED
#define HAL_SD_MODULE_ENABLED

#include "../common/stm32_hal_conf_base.h"


#endif /* __STM32F4xx_HAL_CONF_H */


