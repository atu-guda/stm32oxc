# common Makefile part for STM32F407ZET black board

MCTYPE = STM32F407
MCINCTYPE = $(MCTYPE)xx
FREERTOS_ARCHNAME = ARM_CM4F
BOARDNAME = STM32F407Z-black0
BSPMAKEFILE= $(OXCDIR)/mk/bsp/stm32f407z_black0.mk

#LDSCRIPT = $(STMLD)/STM32F407ZETx_FLASH.ld
# TODO move to common place
LDSCRIPT = $(OXCDIR)/ld/STM32F407ZETx_FLASH.ld
HSE_VALUE = 8000000

SRCS += system_stm32f4xx.c
SRCS += startup_stm32f407xx.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  USE_OXC_CONSOLE_UART = y
  $(info Auto: USE_OXC_CONSOLE_UART)
  # USE_OXC_CONSOLE_USB_CDC = y
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f4_clock_HSE168.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32f4xx_hal.c
  SRCS += stm32f4xx_hal_cortex.c
  SRCS += stm32f4xx_hal_gpio.c
  SRCS += stm32f4xx_hal_rcc.c
endif


