# common Makefile part for STM32F407VGT discovery board

MCTYPE = STM32F407
FREERTOS_ARCHNAME = ARM_CM4F
BOARDNAME = STM32F4-Discovery
BSPNAME   = STM32F4-Discovery

LDSCRIPT = F407XG.ld
HSE_VALUE = 8000000

SRCS += system_stm32f4xx.c
SRCS += startup_stm32f407xx.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  # USE_OXC_CONSOLE_UART = y
  USE_OXC_CONSOLE_USB_CDC = y
  $(info Auto: USE_OXC_CONSOLE_USB_CDC)
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


