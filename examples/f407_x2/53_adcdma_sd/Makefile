PROJ_NAME=adcdma_sd

ADDSRC += $(STMSRC)/FatFs

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_USART = n
USE_OXC_CONSOLE_USB_CDC = y
# USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
USE_USB_DEFAULT_CDC = y
REQUIRE_SPECIAL_CLOCK = y


# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc


SRCS  = main.cpp
SRCS += usbfs_init.cpp
SRCS += adc_init.cpp
SRCS += tim2_init.cpp

SRCS += stm32f4_clock_HSE144.c

SRCS += f4_sdio_init_1bit.cpp
SRCS += bsp_driver_sd.c
# SRCS += ccsbcs.c
SRCS += fatfs.c
SRCS += ff.c
SRCS += ff_gen_drv.c
SRCS += diskio.c
SRCS += sd_diskio.c

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f4_modules.mk

SRCS += stm32f4xx_hal_sd.c
SRCS += stm32f4xx_ll_sdmmc.c

SRCS += stm32f4xx_hal_adc.c
SRCS += stm32f4xx_hal_adc_ex.c
SRCS += stm32f4xx_hal_tim.c
SRCS += stm32f4xx_hal_tim_ex.c
SRCS += stm32f4xx_hal_dma.c

# USB modules added automaticaly  if USE_OXC_CONSOLE_USB_CDC = y

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

