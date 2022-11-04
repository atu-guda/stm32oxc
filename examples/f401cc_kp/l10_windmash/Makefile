OXCDIR = /usr/share/stm32oxc
PROJ_NAME=windmash

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_CONSOLE_UART = y
USE_OXC_CONSOLE_USB_CDC = y
# USE_OXC_CONSOLE_DEFAULT = y
USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_OXC_TIM = y
# USE_OXC_ADC = y
# USE_OXC_DAC = y
USE_OXC_DMA = y
USE_OXC_UART = y
# USE_OXC_SD  = y
# USE_OXC_SDFAT = y
# USE_FREERTOS = y
# NOUSE_OXC_OSFUN = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
# REQUIRE_SPECIAL_CLOCK = y


SRCS += main.cpp
#SRCS += uart_wm.cpp

SRCS += oxc_tmc2209.cpp

SRCS += oxc_usartio.cpp
SRCS += oxc_floatfun.cpp
#SRCS += oxc_statdata.cpp


# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/h7_modules.mk

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

