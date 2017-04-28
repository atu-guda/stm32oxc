PROJ_NAME=adcdma

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_USART = y
USE_OXC_CONSOLE_USB_CDC = n
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y


# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc


SRCS  = main.cpp
SRCS += uart1_init_b6b7.cpp
SRCS += adc_init.cpp
SRCS += tim2_init.cpp


# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f7_modules.mk
SRCS += stm32f7xx_hal_adc.c
SRCS += stm32f7xx_hal_adc_ex.c
SRCS += stm32f7xx_hal_tim.c
SRCS += stm32f7xx_hal_tim_ex.c

SRCS += stm32f7xx_hal_uart.c
SRCS += stm32f7xx_hal_dma.c

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

