#ifndef __OXC_HAL_CONF_AUTO_H
#define __OXC_HAL_CONF_AUTO_H

// automatic converter from OXC_USE define to HAL_XXX_MODULE_ENABLED

#if __has_include("local_hal_conf.h")
  #include "local_hal_conf.h"
#endif

#if __has_include("stm32_hal_conf_base.h")
  #include "stm32_hal_conf_base.h"
#elif __has_include("../common/stm32_hal_conf_base.h")
  #include "../common/stm32_hal_conf_base.h"
#endif

// to prevent warnings
#ifndef USE_HAL_UART_REGISTER_CALLBACKS
  #define USE_HAL_UART_REGISTER_CALLBACKS 0
#endif


#ifndef OXC_SYSTICK_PRTY
  #define OXC_SYSTICK_PRTY 15
#endif

#ifndef NO_COMMON_HAL_MODULES
  #define HAL_MODULE_ENABLED
  #define HAL_CORTEX_MODULE_ENABLED
  #define HAL_RCC_MODULE_ENABLED
  #define HAL_GPIO_MODULE_ENABLED
  #define HAL_FLASH_MODULE_ENABLED
  #define HAL_PWR_MODULE_ENABLED
#endif


// TODO: UART + DMA, PCD(USB) + DMA

#ifdef USE_OXC_CONSOLE_UART
  #ifndef USE_OXC_UART
    #define USE_OXC_UART
  #endif
  #ifndef USE_OXC_CONSOLE
    #define USE_OXC_CONSOLE
  #endif
#endif

#ifdef USE_OXC_UART
  #define HAL_UART_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_CONSOLE_USB_CDC
  #define USE_OXC_USB_CDC
  #define HAL_PCD_MODULE_ENABLED
  #ifndef USE_OXC_CONSOLE
    #define USE_OXC_CONSOLE
  #endif
#endif

#ifdef USE_OXC_I2C
  #define HAL_I2C_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_SPI
  #define HAL_SPI_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_TIM
  #define HAL_TIM_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_ADC
  #define HAL_ADC_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_DAC
  #define HAL_DAC_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_SD
  #define HAL_SD_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_SDRAM
  #define HAL_SDRAM_MODULE_ENABLED
  #define HAL_DMA_MODULE_ENABLED
#endif

#ifdef USE_OXC_DMA
  #ifndef HAL_DMA_MODULE_ENABLED
    #define HAL_DMA_MODULE_ENABLED
  #endif
#endif



#if defined (STM32F0)
 #include <oxc_stm32f0xx_hal_common_conf.h>
#elif defined (STM32F1)
 #include <oxc_stm32f1xx_hal_common_conf.h>
#elif defined (STM32F2)
 #include <oxc_stm32f2xx_hal_common_conf.h>
#elif defined (STM32F3)
 #include <oxc_stm32f3xx_hal_common_conf.h>
#elif defined (STM32F4)
 #include <oxc_stm32f4xx_hal_common_conf.h>
#elif defined(STM32F7)
 #include <oxc_stm32f7xx_hal_common_conf.h>
#else
  #error "Unsupported MCU"
#endif


#endif /* __OXC_HAL_CONF_AUTO_H */


