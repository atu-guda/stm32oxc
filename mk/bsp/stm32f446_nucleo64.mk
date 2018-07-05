# common Makefile part for STM32F446R nucleo64 board

MCTYPE = STM32F446
MCINCTYPE = $(MCTYPE)xx
FREERTOS_ARCHNAME = ARM_CM4F
BOARDNAME = STM32F4xx-Nucleo   # see /usr/share/stm32cube/bsp/
BSPMAKEFILE= $(OXCDIR)/mk/bsp/stm32f446_nucleo64.mk   # self, for deps

LDSCRIPT = $(STMLD)/STM32F446RETx_FLASH.ld
HSE_VALUE = 8000000

SRCS += system_stm32f4xx.c
SRCS += startup_stm32f446xx.s

ifneq "$(OXC_NO_STD_CONSOLE)" "y"
  USE_OXC_CONSOLE_UART = y
  # USE_OXC_CONSOLE_USB_CDC = y
endif

ifneq "$(REQUIRE_SPECIAL_CLOCK)" "y"
  SRCS += stm32f446_clock_HSE168.c
endif

# base hal files
ifeq "$(NO_COMMON_HAL_MODULES)" "y"
  SRCS += stm32f4xx_hal.c
  SRCS += stm32f4xx_hal_cortex.c
  SRCS += stm32f4xx_hal_gpio.c
  SRCS += stm32f4xx_hal_rcc.c
  SRCS += stm32f4xx_hal_rcc_ex.c
  SRCS += stm32f4xx_hal_pwr_ex.c
endif


