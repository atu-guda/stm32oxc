# common Makefile part for STM32F429discovery

MCTYPE = STM32F429
MCINCTYPE = $(MCTYPE)xx
FREERTOS_ARCHNAME = ARM_CM4F
BOARDNAME = STM32F429I-Discovery

ADDSRC += ../common ../../common

#
LDSCRIPT = $(STMLD)/STM32F429ZITx_FLASH.ld
HSE_VALUE = 8000000

SRCS += system_stm32f4xx.c
SRCS += startup_stm32f429xx.s

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  # SRCS += stm32f4_clock_HSE180.c
  SRCS += stm32f4_clock_HSE192.c
endif

# base hal files
SRCS += stm32f4xx_hal.c
SRCS += stm32f4xx_hal_cortex.c
SRCS += stm32f4xx_hal_gpio.c
SRCS += stm32f4xx_hal_rcc.c

ifeq "$(USE_USB)" "y"
  SRCS += usbfs_init.cpp
endif


