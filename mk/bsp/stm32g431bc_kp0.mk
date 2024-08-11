# common Makefile part for STM32G431BCU blacK pill board

MCTYPE = STM32G431
MCINCTYPE = STM32G431xx
FREERTOS_ARCHNAME = ARM_CM4F
BSPNAME   = STM32G431bc_kp0

LDSCRIPT = G431XB.ld
HSE_VALUE = 8000000

SRCS += system_stm32g4xx.c
SRCS += startup_stm32g431xx.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  # USE_OXC_CONSOLE_UART = y
  USE_OXC_CONSOLE_USB_CDC = y
  $(info Auto: USE_OXC_CONSOLE_USB_CDC)
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32g431_clock_HSE144.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32g4xx_hal.c
  SRCS += stm32g4xx_hal_cortex.c
  SRCS += stm32g4xx_hal_gpio.c
  SRCS += stm32g4xx_hal_rcc.c
  SRCS += stm32g4xx_hal_rcc_ex.c
  SRCS += stm32g4xx_hal_pwr.c
  SRCS += stm32g4xx_hal_pwr_ex.c
endif


