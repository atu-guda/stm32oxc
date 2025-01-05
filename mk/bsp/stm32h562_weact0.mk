# common Makefile part for STM32H562RGT WeAct board

MCTYPE = STM32H562
FREERTOS_ARCHNAME = ARM_CM4F
# BOARDNAME = STM32FH562-weact0
BSPNAME   = STM32H562-weact0

LDSCRIPT = H562XX.ld
HSE_VALUE = 8000000

SRCS += system_stm32h5xx.c
SRCS += startup_stm32h562xx.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  USE_OXC_CONSOLE_UART = y
  # USE_OXC_CONSOLE_USB_CDC = y
  # $(info Auto: USE_OXC_CONSOLE_USB_CDC)
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32h5_clock_HSE200_8.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32h5xx_hal.c
  SRCS += stm32h5xx_hal_cortex.c
  SRCS += stm32h5xx_hal_gpio.c
  SRCS += stm32h5xx_hal_rcc.c
  SRCS += stm32h5xx_hal_rcc_ex.c
  SRCS += stm32h5xx_hal_pwr.c
  SRCS += stm32h5xx_hal_pwr_ex.c
  SRCS += stm32h5xx_hal_icache.c
endif


