PROJ_NAME=sdcard2

ADDSRC += $(STMSRC)/FatFs

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_USART = y
USE_OXC_CONSOLE_USB_CDC = n
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y
# REQUIRE_SPECIAL_CLOCK = y


# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc


SRCS  = main.cpp
SRCS += uart2_init_a2a3.cpp

SRCS += f4_sdio_init_1bit.cpp
SRCS += bsp_driver_sd.c
SRCS += fatfs.c
SRCS += ff.c
SRCS += ff_gen_drv.c
SRCS += diskio.c
SRCS += sd_diskio.c

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# list: see ../common/f4_modules.mk
SRCS += stm32f4xx_hal_sd.c
SRCS += stm32f4xx_ll_sdmmc.c



include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

