# common Makefile part for STM32F407VET X2 board

MCTYPE = STM32F407
MCINCTYPE = $(MCTYPE)xx

vpath %.cpp ../common ../../common

#
LINK=$(CXX)
LDSCRIPT = $(STMLD)/STM32F407VE_FLASH.ld
HSE_VALUE = 8000000

SRCS += system_stm32f4xx.c
SRCS += startup_stm32f407xx.s

SRCS += stm32f4_clock_HSE168.c

# base hal files
SRCS += stm32f4xx_hal.c
SRCS += stm32f4xx_hal_cortex.c
SRCS += stm32f4xx_hal_gpio.c
SRCS += stm32f4xx_hal_rcc.c

ifeq "$(USE_USB)" "y"
  SRCS += usbfs_init.cpp
endif

ifeq "$(USE_OXC_CONSOLE_USB_CDC)"  "y"
  SRCS += stm32f4xx_hal_pcd.c
  SRCS += stm32f4xx_ll_usb.c
  SRCS += stm32f4xx_hal_pcd_ex.c
endif

