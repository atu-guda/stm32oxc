# common Makefile part for STM32H743GI WeAct board

MCTYPE = STM32H743
FREERTOS_ARCHNAME = ARM_CM4F
# BOARDNAME = STM32FH743-weact0
BSPNAME   = STM32H743-weact0

LDSCRIPT = H743XI.ld
HSE_VALUE = 25000000

SRCS += system_stm32h7xx.c
SRCS += startup_stm32h743xx.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  # USE_OXC_CONSOLE_UART = y
  USE_OXC_CONSOLE_USB_CDC = y
  # $(info Auto: USE_OXC_CONSOLE_USB_CDC)
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32h7_clock_HSE400_25.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32h7xx_hal.c
  SRCS += stm32h7xx_hal_cortex.c
  SRCS += stm32h7xx_hal_gpio.c
  SRCS += stm32h7xx_hal_rcc.c
  SRCS += stm32h7xx_hal_rcc_ex.c
  SRCS += stm32h7xx_hal_pwr.c
  SRCS += stm32h7xx_hal_pwr_ex.c
endif


