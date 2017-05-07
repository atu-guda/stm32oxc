PROJ_NAME=accel_test

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_USART = y
USE_OXC_CONSOLE_USB_CDC = n
USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y
# REQUIRE_SPECIAL_CLOCK = y


OXCDIR = /usr/share/stm32oxc

CFLAGS_ADD += -I$(STMDIR)/bsp/Components/lsm303dlhc
ADDSRC     +=   $(STMDIR)/bsp/Components/lsm303dlhc

SRCS  = main.cpp

SRCS += oxc_i2c_default_init.cpp


# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f3_modules.mk

SRCS += stm32f3xx_hal_i2c_ex.c

SRCS += lsm303dlhc.c



include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

