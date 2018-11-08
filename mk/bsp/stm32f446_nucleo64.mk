# common Makefile part for STM32F446R nucleo64 board

MCTYPE = STM32F446
MCINCTYPE = $(MCTYPE)xx
FREERTOS_ARCHNAME = ARM_CM4F
# see /usr/share/stm32cube/bsp/
BOARDNAME = STM32F4xx-Nucleo

LDSCRIPT = F446XE.ld
HSE_VALUE = 8000000

SRCS += system_stm32f4xx.c
SRCS += startup_stm32f446xx.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  USE_OXC_CONSOLE_UART = y
  $(info Auto: USE_OXC_CONSOLE_UART)
  # USE_OXC_CONSOLE_USB_CDC = y
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f446_clock_HSE168.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32f4xx_hal.c
  SRCS += stm32f4xx_hal_cortex.c
  SRCS += stm32f4xx_hal_gpio.c
  SRCS += stm32f4xx_hal_rcc.c
  SRCS += stm32f4xx_hal_rcc_ex.c
  SRCS += stm32f4xx_hal_pwr_ex.c
endif


