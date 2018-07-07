#ifndef __STM32_HAL_CONF_H
#define __STM32_HAL_CONF_H

// base configuaration file for oxc f407 x2 board examples

#define REQ_MCBASE STM32F4
#ifndef REQ_SYSCLK_FREQ
  #define REQ_SYSCLK_FREQ 168
#endif


#include <bsp/board_stm32f4discovery.h>



#endif /* __STM32_HAL_CONF_H */


