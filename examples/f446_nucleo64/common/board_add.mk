# common Makefile part for STM32F446R nucleo64 board

MCTYPE = STM32F446
MCINCTYPE = $(MCTYPE)xx
FREERTOS_ARCHNAME = ARM_CM4F
BOARDNAME = STM32F4xx-Nucleo

vpath %.cpp ../common ../../common

#
LDSCRIPT = $(STMLD)/STM32F446RETx_FLASH.ld
HSE_VALUE = 8000000

SRCS += system_stm32f4xx.c
SRCS += startup_stm32f446xx.s

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f446_clock_HSE192.c
endif

# base hal files
SRCS += stm32f4xx_hal.c
SRCS += stm32f4xx_hal_cortex.c
SRCS += stm32f4xx_hal_gpio.c
SRCS += stm32f4xx_hal_rcc.c
SRCS += stm32f4xx_hal_rcc_ex.c
SRCS += stm32f4xx_hal_pwr_ex.c

ifeq "$(USE_USB)" "y"
  SRCS += usbfs_init.cpp
endif


