PROJ_NAME=uart

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_UART = y
USE_OXC_CONSOLE_USB_CDC = n
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# REQUIRE_SPECIAL_CLOCK = y


OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

# for now
NOUSE_DEFAULT_UART_INIT = y
SRCS += stm32f1xx_hal_msp.cpp

SRCS += oxc_common1.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# list: see ../common/f1_modules.mk

SRCS +=  stm32f1xx_hal_uart.c

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

