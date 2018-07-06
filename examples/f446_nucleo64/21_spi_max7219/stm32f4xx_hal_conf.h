#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

// ########################## Module Selection ##############################
// base modules

// here to define REQ_MCBASE (STM32Fx), REQ_SYSCLK_FREQ,
// and include <bsp/board_stm32XXX_XXX.h>
#include "../common/stm32_hal_conf_base.h"

// AUX modules : full list: ../common/hal_modules_list.h
//

#include "local_hal_conf.h" // if not exist, included from $(OXCDIR)/inc

#include <oxc_hal_conf_auto.h>


#endif /* __STM32F4xx_HAL_CONF_H */


