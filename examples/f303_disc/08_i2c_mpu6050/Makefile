PROJ_NAME=i2c_mpu6050

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_USART = y
USE_OXC_CONSOLE_USB_CDC = n
USE_OXC_I2C = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y
# REQUIRE_SPECIAL_CLOCK = y


# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc


SRCS  = main.cpp
SRCS += uart2_init_a2a3.cpp
SRCS += i2c1_b6b7_hw.cpp

SRCS += oxc_mpu6050.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# list: see ../common/f3_modules.mk


SRCS += stm32f3xx_hal_i2c_ex.c


include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

