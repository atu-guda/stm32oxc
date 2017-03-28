# common Makefile part for STM32F103C8 small board oxc examples.

MCTYPE = STM32F103
MCINCTYPE = STM32F103xB # realy C8
FREERTOS_ARCHNAME = ARM_CM3
BOARDNAME = STM32F103-BluePill # unofficial

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

