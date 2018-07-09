PROJ_NAME=uart

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_UART = y
USE_OXC_CONSOLE_USB_CDC = n
# USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# REQUIRE_SPECIAL_CLOCK = y

# NOUSE_DEFAULT_UART_INIT = y

OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f7_modules.mk



include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

