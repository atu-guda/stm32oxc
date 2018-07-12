# common Makefile part for STM32F103C8 small board oxc examples.

MCTYPE = STM32F103
# realy C8
MCINCTYPE = STM32F103xB
FREERTOS_ARCHNAME = ARM_CM3
# unofficial
BOARDNAME = STM32F103-BluePill

ADDSRC += ../common ../../common

#
LDSCRIPT = $(STMLD)/STM32F103X8_FLASH.ld
HSE_VALUE = 8000000

SRCS += stm32f1xx_hal_msp.c
SRCS += system_stm32f1xx.c
SRCS += startup_stm32f103xb.s

SRCS += stm32f1_clock_HSE072.c
# SRCS += stm32f1_clock_HSE072_LSE.c

# base hal files
SRCS += stm32f1xx_hal.c
SRCS += stm32f1xx_hal_cortex.c
SRCS += stm32f1xx_hal_gpio.c
SRCS += stm32f1xx_hal_rcc.c

