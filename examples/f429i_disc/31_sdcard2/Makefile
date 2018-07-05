PROJ_NAME=sdcard2

ADDSRC += $(STMSRC)/FatFs

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_UART = y
USE_OXC_CONSOLE_USB_CDC = n
# USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y
# REQUIRE_SPECIAL_CLOCK = y
USE_DEFAULT_SDIO_FAT = y


OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

SRCS += oxc_fs_cmd0.cpp

SRCS += f4_sdio_init_1bit.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f4_modules.mk

SRCS += stm32f4xx_hal_sd.c
SRCS += stm32f4xx_ll_sdmmc.c



include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

