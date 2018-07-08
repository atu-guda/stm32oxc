# common Makefile part for STM32F746IG WaveShare board

MCTYPE = STM32F746
MCINCTYPE = $(MCTYPE)xx
FREERTOS_ARCHNAME = ARM_CM4F
# none
BOARDNAME = STM32F746-wave0
BSPMAKEFILE= $(OXCDIR)/mk/bsp/stm32f746_wave.mk

LDSCRIPT = $(STMLD)/STM32F746ZGTx_FLASH.ld
HSE_VALUE = 8000000

SRCS += system_stm32f7xx.c
SRCS += startup_stm32f746xx.s

ifeq "$(USE_OXC_CONSOLE_DEFAULT)" "y"
  USE_OXC_CONSOLE_UART = y
  # USE_OXC_CONSOLE_USB_CDC = y
  # $(info Auto: USE_OXC_CONSOLE_USB_CDC)
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f7_clock_HSE200.c
endif

# base hal files
ifneq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32f7xx_hal.c
  SRCS += stm32f7xx_hal_cortex.c
  SRCS += stm32f7xx_hal_gpio.c
  SRCS += stm32f7xx_hal_rcc.c
  SRCS += stm32f7xx_hal_rcc_ex.c
  SRCS += stm32f7xx_hal_pwr.c
  SRCS += stm32f7xx_hal_pwr_ex.c
endif


