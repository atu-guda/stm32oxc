# common Makefile part for STM32F407VET X2 board

MCTYPE = STM32F427
MCINCTYPE = $(MCTYPE)xx
FREERTOS_ARCHNAME = ARM_CM4F
BOARDNAME = STM32F427-netdu # none

ADDSRC += ../common ../../common

#
#LDSCRIPT = $(STMLD)/STM32F427VITx_FLASH.ld
# TMP, untill add
LDSCRIPT = $(STMLD)/STM32F417IG_FLASH.ld
HSE_VALUE = 25000000

SRCS += system_stm32f4xx.c
SRCS += startup_stm32f427xx.s

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f4_clock_HSE168_25.c
endif

# base hal files
SRCS += stm32f4xx_hal.c
SRCS += stm32f4xx_hal_cortex.c
SRCS += stm32f4xx_hal_gpio.c
SRCS += stm32f4xx_hal_rcc.c


