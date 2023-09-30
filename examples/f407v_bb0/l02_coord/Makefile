OXCDIR = /usr/share/stm32oxc

PROJ_NAME=coord0

USE_OXC = y
USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_UART = y
# USE_OXC_CONSOLE_USB_CDC = y
# USE_OXC_CONSOLE_DEFAULT = y
USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_OXC_TIM = y
USE_OXC_ADC = y
# USE_OXC_DAC = y
USE_OXC_DMA = y
USE_OXC_SD  = y
USE_OXC_SDFAT = y
# USE_OXC_MSCFAT = y
# USE_FREERTOS = y
# USE_OXC_PICOC = y
# NOUSE_OXC_OSFUN = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
# REQUIRE_SPECIAL_CLOCK = y

# NOUSE_DEFAULT_UART_INIT = y

SRCS  += main.cpp
SRCS  += oxc_gcode.cpp
SRCS  += tim_funcs.cpp
# SRCS  += gcodes.cpp

SRCS += oxc_floatfun.cpp
#SRCS += oxc_namedobjs.cpp
#SRCS += oxc_namedints.cpp
#SRCS += oxc_namedfloats.cpp
SRCS += f4_sdio_init_1bit.cpp
#SRCS += oxc_outstr.cpp
#SRCS += oxc_hd44780_i2c.cpp
#SRCS += oxc_menu4b.cpp
#SRCS += oxc_statdata.cpp
#SRCS += oxc_ds3231.cpp

# SRCS += oxc_hd44780_i2c.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f4_modules.mk

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

