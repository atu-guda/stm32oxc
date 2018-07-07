#ifndef __STM32_HAL_CONF_BASE_H
#define __STM32_HAL_CONF_BASE_H

// base configuaration file for oxc f303 discovery examples

#define REQ_MCBASE STM32F3
#ifndef REQ_SYSCLK_FREQ
  #define REQ_SYSCLK_FREQ 72
#endif


#include <bsp/board_stm32f3discovery.h>



#endif // __STM32_HAL_CONF_BASE_H


