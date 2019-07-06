# common Makefile part for STM32F334R nucleo64 board

MCTYPE = STM32F334
MCINCTYPE = $(MCTYPE)x8
FREERTOS_ARCHNAME = ARM_CM4F
# see /usr/share/stm32cube/bsp/
BOARDNAME = STM32F3xx-Nucleo

LDSCRIPT = F334X8.ld
HSE_VALUE = 8000000

SRCS += system_stm32f3xx.c
SRCS += startup_stm32f334x8.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  USE_OXC_CONSOLE_UART = y
  $(info Auto: USE_OXC_CONSOLE_UART)
  # USE_OXC_CONSOLE_USB_CDC = y
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f334_clock_HSE072.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32f3xx_hal.c
  SRCS += stm32f3xx_hal_cortex.c
  SRCS += stm32f3xx_hal_gpio.c
  SRCS += stm32f3xx_hal_rcc.c
  SRCS += stm32f3xx_hal_rcc_ex.c
  SRCS += stm32f3xx_hal_pwr_ex.c
endif


