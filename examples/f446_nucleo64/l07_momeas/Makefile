OXCDIR = /usr/share/stm32oxc

PROJ_NAME=momeas

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_CONSOLE_UART = y
# USE_OXC_CONSOLE_USB_CDC = y
USE_OXC_CONSOLE_DEFAULT = y
USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_OXC_TIM = y
# USE_OXC_ADC = y
# USE_OXC_DAC = y
USE_OXC_DMA = y
# USE_OXC_SD  = y
# USE_OXC_SDFAT = y
# USE_FREERTOS = y
# NOUSE_OXC_OSFUN = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
# REQUIRE_SPECIAL_CLOCK = y
USE_OXC_MODBUS_RTU = y

# NOUSE_DEFAULT_UART_INIT = y

SRCS += main.cpp
SRCS += oxc_namedobjs.cpp
SRCS += oxc_namedfloats.cpp
SRCS += oxc_statdata.cpp
SRCS += oxc_hx711.cpp
SRCS += tim.cpp
SRCS += oxc_modbus_rd6006.cpp
SRCS += uart_for_modbus.cpp

SRCS += oxc_floatfun.cpp



# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f4_modules.mk

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

