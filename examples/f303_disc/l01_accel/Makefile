OXCDIR = /usr/share/stm32oxc

PROJ_NAME=accel_test

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_CONSOLE_UART = y
# USE_OXC_CONSOLE_USB_CDC = y
USE_OXC_CONSOLE_DEFAULT = y
USE_OXC_I2C = y
# USE_OXC_SPI = y
# USE_OXC_TIM = y
# USE_OXC_ADC = y
# USE_OXC_DAC = y
# USE_OXC_DMA = y
# USE_OXC_SD  = y
# USE_OXC_SDFAT = y
USE_FREERTOS = y
# NOUSE_OXC_OSFUN = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
# REQUIRE_SPECIAL_CLOCK = y


CFLAGS_ADD += -I$(STMDIR)/bsp/Components/lsm303dlhc
ADDSRC     +=   $(STMDIR)/bsp/Components/lsm303dlhc

SRCS  = main.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f3_modules.mk

SRCS += stm32f3xx_hal_i2c_ex.c

SRCS += lsm303dlhc.c


include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

