PROJ_NAME=spi_test

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_USART = n
USE_OXC_CONSOLE_USB_CDC = y
USE_OXC_SPI = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
USE_USB_DEFAULT_CDC = y
# REQUIRE_SPECIAL_CLOCK = y


# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc


SRCS  = main.cpp
SRCS += usbfs_init.cpp
SRCS += init_spi1_common_master_a567.cpp


# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# list: see ../common/f4_modules.mk



include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

