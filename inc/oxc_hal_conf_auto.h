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
// if another value is needed, use local_hal_conf.h

#ifndef USE_HAL_UART_REGISTER_CALLBACKS
  #define USE_HAL_UART_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_USART_REGISTER_CALLBACKS
  #define USE_HAL_USART_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_CAN_REGISTER_CALLBACKS
  #define USE_CAN_UART_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_CEC_REGISTER_CALLBACKS
  #define USE_CEC_UART_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_I2C_REGISTER_CALLBACKS
  #define USE_HAL_I2C_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_SPI_REGISTER_CALLBACKS
  #define USE_HAL_SPI_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_DMA2D_REGISTER_CALLBACKS
  #define USE_HAL_DMA2D_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_OPAMP_REGISTER_CALLBACKS
  #define USE_HAL_OPAMP_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_TIM_REGISTER_CALLBACKS
  #define USE_HAL_TIM_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_HRTIM_REGISTER_CALLBACKS
  #define USE_HAL_HRTIM_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_ADC_REGISTER_CALLBACKS
  #define USE_HAL_ADC_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_CAN_REGISTER_CALLBACKS
  #define USE_HAL_CAN_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_CEC_REGISTER_CALLBACKS
  #define USE_HAL_CEC_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_CRYP_REGISTER_CALLBACKS
  #define USE_HAL_CRYP_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_DAC_REGISTER_CALLBACKS
  #define USE_HAL_DAC_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_DCMI_REGISTER_CALLBACKS
  #define USE_HAL_DCMI_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_DFSDM_REGISTER_CALLBACKS
  #define USE_HAL_DFSDM_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_DMA2D_REGISTER_CALLBACKS
  #define USE_HAL_DMA2D_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_DSI_REGISTER_CALLBACKS
  #define USE_HAL_DSI_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_ETH_REGISTER_CALLBACKS
  #define USE_HAL_ETH_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_HASH_REGISTER_CALLBACKS
  #define  USE_HAL_HASH_REGISTER_CALLBACKS        0
#endif

#ifndef USE_HAL_I2S_REGISTER_CALLBACKS
  #define  USE_HAL_I2S_REGISTER_CALLBACKS         0
#endif

#ifndef USE_HAL_IRDA_REGISTER_CALLBACKS
  #define  USE_HAL_IRDA_REGISTER_CALLBACKS        0
#endif

#ifndef USE_HAL_JPEG_REGISTER_CALLBACKS
  #define  USE_HAL_JPEG_REGISTER_CALLBACKS        0
#endif

#ifndef USE_HAL_LPTIM_REGISTER_CALLBACKS
  #define  USE_HAL_LPTIM_REGISTER_CALLBACKS       0
#endif

#ifndef USE_HAL_MDIOS_REGISTER_CALLBACKS
  #define  USE_HAL_MDIOS_REGISTER_CALLBACKS       0
#endif

#ifndef USE_HAL_MMC_REGISTER_CALLBACKS
  #define  USE_HAL_MMC_REGISTER_CALLBACKS         0
#endif

#ifndef USE_HAL_NAND_REGISTER_CALLBACKS
  #define  USE_HAL_NAND_REGISTER_CALLBACKS        0
#endif

#ifndef USE_HAL_NOR_REGISTER_CALLBACKS
  #define  USE_HAL_NOR_REGISTER_CALLBACKS         0
#endif

#ifndef USE_HAL_QSPI_REGISTER_CALLBACKS
  #define  USE_HAL_QSPI_REGISTER_CALLBACKS        0
#endif

#ifndef USE_HAL_RNG_REGISTER_CALLBACKS
  #define  USE_HAL_RNG_REGISTER_CALLBACKS         0
#endif

#ifndef USE_HAL_RTC_REGISTER_CALLBACKS
  #define  USE_HAL_RTC_REGISTER_CALLBACKS         0
#endif

#ifndef USE_HAL_SAI_REGISTER_CALLBACKS
  #define  USE_HAL_SAI_REGISTER_CALLBACKS         0
#endif

#ifndef USE_HAL_SMARTCARD_REGISTER_CALLBACKS
  #define  USE_HAL_SMARTCARD_REGISTER_CALLBACKS   0
#endif

#ifndef USE_HAL_SRAM_REGISTER_CALLBACKS
  #define  USE_HAL_SRAM_REGISTER_CALLBACKS        0
#endif

#ifndef USE_HAL_SPDIFRX_REGISTER_CALLBACKS
  #define  USE_HAL_SPDIFRX_REGISTER_CALLBACKS     0
#endif

#ifndef USE_HAL_SMBUS_REGISTER_CALLBACKS
  #define  USE_HAL_SMBUS_REGISTER_CALLBACKS       0
#endif

#ifndef USE_HAL_WWDG_REGISTER_CALLBACKS
  #define  USE_HAL_WWDG_REGISTER_CALLBACKS        0
#endif

#ifndef USE_HAL_SDADC_REGISTER_CALLBACKS
  #define USE_HAL_SDADC_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_DAC_REGISTER_CALLBACKS
  #define USE_HAL_DAC_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_LTDC_REGISTER_CALLBACKS
  #define USE_HAL_LTDC_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_SD_REGISTER_CALLBACKS
  #define USE_HAL_SD_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_SDRAM_REGISTER_CALLBACKS
  #define USE_HAL_SDRAM_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_PCD_REGISTER_CALLBACKS
  #define USE_HAL_PCD_REGISTER_CALLBACKS 0
#endif

#ifndef USE_HAL_HCD_REGISTER_CALLBACKS
  #define USE_HAL_HCD_REGISTER_CALLBACKS 0
#endif

#ifndef USBD_USER_REGISTER_CALLBACK
  #define USBD_USER_REGISTER_CALLBACK 0
#endif

#ifndef ART_ACCELERATOR_ENABLE
  #define ART_ACCELERATOR_ENABLE 0
#endif

#ifndef USE_SPI_CRC
  #define USE_SPI_CRC 0
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

#ifdef USE_USBD_LIB
  #define HAL_PCD_MODULE_ENABLED
#endif

#ifdef USE_OXC_CONSOLE_USB_CDC
  #define USE_OXC_USB_CDC
  #ifndef USE_OXC_CONSOLE
    #define USE_OXC_CONSOLE
  #endif
#endif

#ifdef USE_USBH_LIB
  #define HAL_HCD_MODULE_ENABLED
  #define HAL_PCD_MODULE_ENABLED
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
  #if defined(STM32H7)
    #define HAL_MDMA_MODULE_ENABLED
  #endif
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
#elif defined(STM32H7)
 #include <oxc_stm32h7xx_hal_common_conf.h>
#elif defined(STM32G4)
 #include <oxc_stm32g4xx_hal_common_conf.h>
#else
  #error "Unsupported MCU"
#endif


#endif /* __OXC_HAL_CONF_AUTO_H */


