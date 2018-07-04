PROJ_NAME=usb_console

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_CONSOLE_USART = y
USE_OXC_CONSOLE_USB_CDC = y
# USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# REQUIRE_SPECIAL_CLOCK = y


OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

SRCS += empty_test0_function.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f4_modules.mk


include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

