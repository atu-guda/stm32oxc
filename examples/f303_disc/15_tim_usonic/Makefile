PROJ_NAME=tim8_usonic

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
SRCS += uart2_a2a3_hw.cpp
SRCS += tim8_init_hw.cpp


# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# list: see ../common/f3_modules.mk

SRCS += stm32f3xx_hal_tim.c
SRCS += stm32f3xx_hal_tim_ex.c



include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

