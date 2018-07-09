PROJ_NAME=leds

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = n
USE_OXC_CONSOLE_UART = n
USE_OXC_CONSOLE_USB_CDC = n
USE_FREERTOS = n
NOUSE_OXC_OSFUN = y
# FREERTOS_HEAP = heap_2.c
# REQUIRE_SPECIAL_CLOCK = y


OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f7_modules.mk



include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

