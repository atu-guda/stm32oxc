PROJ_NAME=adcdma_fmc

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_UART = y
USE_OXC_CONSOLE_USB_CDC = n
# USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y
REQUIRE_SPECIAL_CLOCK = y

OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

SRCS += adc_init.cpp
SRCS += tim2_init.cpp
SRCS += fmc_sdram_init.cpp

SRCS += stm32f7_clock_HSE144.c


# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f7_modules.mk
SRCS += stm32f7xx_hal_adc.c
SRCS += stm32f7xx_hal_adc_ex.c
SRCS += stm32f7xx_hal_tim.c
SRCS += stm32f7xx_hal_tim_ex.c

SRCS += stm32f7xx_hal_dma.c
SRCS += stm32f7xx_hal_sdram.c
SRCS += stm32f7xx_ll_fmc.c

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

