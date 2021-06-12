# common Makefile part for STM32F401CCU blacK pill board

MCTYPE = STM32F401
MCINCTYPE = STM32F401xC
FREERTOS_ARCHNAME = ARM_CM4F
BSPNAME   = STM32F401cc_kp

LDSCRIPT = F401XC.ld
HSE_VALUE = 25000000

SRCS += system_stm32f4xx.c
SRCS += startup_stm32f401xc.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  # USE_OXC_CONSOLE_UART = y
  USE_OXC_CONSOLE_USB_CDC = y
  $(info Auto: USE_OXC_CONSOLE_USB_CDC)
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f401_clock_HSE72_25.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32f4xx_hal.c
  SRCS += stm32f4xx_hal_cortex.c
  SRCS += stm32f4xx_hal_gpio.c
  SRCS += stm32f4xx_hal_rcc.c
endif


