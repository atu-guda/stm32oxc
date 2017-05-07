# common Makefile part for STM32F407VET X2 board

MCTYPE = STM32F746
MCINCTYPE = $(MCTYPE)xx
FREERTOS_ARCHNAME = ARM_CM4F
BOARDNAME = STM32F746-wave0 # none

ADDSRC += ../common ../../common

#
LINK=$(CXX)
LDSCRIPT = $(STMLD)/STM32F746ZGTx_FLASH.ld
HSE_VALUE = 8000000

SRCS += system_stm32f7xx.c
SRCS += startup_stm32f746xx.s

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f7_clock_HSE200.c
endif

# base hal files
SRCS += stm32f7xx_hal.c
SRCS += stm32f7xx_hal_cortex.c
SRCS += stm32f7xx_hal_gpio.c
SRCS += stm32f7xx_hal_rcc.c
SRCS += stm32f7xx_hal_rcc_ex.c
SRCS += stm32f7xx_hal_pwr.c
SRCS += stm32f7xx_hal_pwr_ex.c


