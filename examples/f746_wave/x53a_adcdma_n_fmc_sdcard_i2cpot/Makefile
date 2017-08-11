PROJ_NAME=adcdma_nfx_pot

ADDSRC += $(STMSRC)/FatFs

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
REQUIRE_SPECIAL_CLOCK = y


OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

SRCS += ff_syncobj.cpp

SRCS += adc_init_exa_4ch_dma_n.cpp

SRCS += f7_adcdma_n.cpp

SRCS += tim2_adcdma_init.cpp

SRCS += stm32f7_clock_HSE144.c

SRCS += f4_sdio_init_1bit.cpp
SRCS += bsp_driver_sd.c
# SRCS += ccsbcs.c
SRCS += fatfs.c
SRCS += ff.c
SRCS += ff_gen_drv.c
SRCS += diskio.c
SRCS += sd_diskio.c

SRCS += fmc_sdram_init.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f7_modules.mk

SRCS += stm32f7xx_hal_sd.c
SRCS += stm32f7xx_ll_sdmmc.c

SRCS += stm32f7xx_hal_adc.c
SRCS += stm32f7xx_hal_adc_ex.c
SRCS += stm32f7xx_hal_tim.c
SRCS += stm32f7xx_hal_tim_ex.c
SRCS += stm32f7xx_hal_dma.c
SRCS += stm32f7xx_hal_dma_ex.c
SRCS += stm32f7xx_hal_sdram.c
SRCS += stm32f7xx_ll_fmc.c


include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

