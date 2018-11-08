# common Makefile part for STM32F103C8T6 bluepill board

MCTYPE = STM32F103
# realy C8
MCINCTYPE = STM32F103xB
FREERTOS_ARCHNAME = ARM_CM3
# unofficial
BOARDNAME = STM32F103-BluePill

LDSCRIPT = F103X8.ld
HSE_VALUE = 8000000

SRCS += system_stm32f1xx.c
SRCS += startup_stm32f103xb.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  USE_OXC_CONSOLE_UART = y
  # USE_OXC_CONSOLE_USB_CDC = y
  # $(info Auto: USE_OXC_CONSOLE_USB_CDC)
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f1_clock_HSE072.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32f1xx_hal.c
  SRCS += stm32f1xx_hal_cortex.c
  SRCS += stm32f1xx_hal_gpio.c
  SRCS += stm32f1xx_hal_rcc.c
  SRCS += stm32f1xx_hal_rcc_ex.c
endif


