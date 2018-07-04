PROJ_NAME=spi_mem_a25

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_USART = y
USE_OXC_CONSOLE_USB_CDC = n
# USE_OXC_I2C = y
USE_OXC_SPI = y
# USE_OXC_TIM = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y
# REQUIRE_SPECIAL_CLOCK = y
# USE_DEFAULT_SDIO_FAT = y


OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

SRCS += oxc_spimem_at.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f4_modules.mk


include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

