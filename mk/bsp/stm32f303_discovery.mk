# common Makefile part for STM32F303VC discovery board

MCTYPE = STM32F303
MCINCTYPE = STM32F303xC
FREERTOS_ARCHNAME = ARM_CM4F
BOARDNAME = STM32F3-Discovery
BSPMAKEFILE= $(OXCDIR)/mk/bsp/stm32f303_discovery.mk

LDSCRIPT = $(STMLD)/STM32F303VC_FLASH.ld
HSE_VALUE = 8000000

SRCS += system_stm32f3xx.c
SRCS += startup_stm32f303xc.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  USE_OXC_CONSOLE_UART = y
  # USE_OXC_CONSOLE_USB_CDC = y
  # $(info Auto: USE_OXC_CONSOLE_USB_CDC)
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f3_clock_HSE072_bc.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32f3xx_hal.c
  SRCS += stm32f3xx_hal_cortex.c
  SRCS += stm32f3xx_hal_gpio.c
  SRCS += stm32f3xx_hal_rcc.c
  SRCS += stm32f3xx_hal_rcc_ex.c
endif


