# common Makefile part for STM32F3Discovery oxc examples.

MCTYPE = STM32F303
MCINCTYPE = STM32F303xC
FREERTOS_ARCHNAME = ARM_CM4F
BOARDNAME = STM32F3-Discovery

vpath %.cpp ../common ../../common

#
LDSCRIPT = $(STMLD)/STM32F303VC_FLASH.ld
HSE_VALUE = 8000000

SRCS += system_stm32f3xx.c
SRCS += startup_stm32f303xc.s

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f3_clock_HSE072b.c
endif

# base hal files
SRCS += stm32f3xx_hal.c
SRCS += stm32f3xx_hal_cortex.c
SRCS += stm32f3xx_hal_gpio.c
SRCS += stm32f3xx_hal_rcc.c
SRCS += stm32f3xx_hal_rcc_ex.c

ifeq "$(USE_USB)" "y"
  SRCS += usbfs_init.cpp
endif


