# common Makefile part for STM32F3Discovery oxc examples.

MCTYPE = STM32F303
MCINCTYPE = STM32F303xC

vpath %.cpp ../common ../../common

#
LINK=$(CXX)
LDSCRIPT = $(STMLD)/STM32F303VC_FLASH.ld
HSE_VALUE = 8000000

SRCS += stm32f3xx_hal_msp.cpp
SRCS += system_stm32f3xx.c
SRCS += startup_stm32f303xc.s

SRCS += stm32f3_clock_HSE072b.c

# base hal files
SRCS += stm32f3xx_hal.c
SRCS += stm32f3xx_hal_cortex.c
SRCS += stm32f3xx_hal_gpio.c
SRCS += stm32f3xx_hal_rcc.c

