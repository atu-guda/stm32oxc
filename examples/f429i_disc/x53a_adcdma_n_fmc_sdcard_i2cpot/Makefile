OXCDIR = /usr/share/stm32oxc
PROJ_NAME=adcdma_nfx_pot

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_CONSOLE_UART = y
# USE_OXC_CONSOLE_USB_CDC = y
USE_OXC_CONSOLE_DEFAULT = y
USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_OXC_TIM = y
USE_OXC_ADC = y
# USE_OXC_DAC = y
USE_OXC_DMA = y
USE_OXC_SD  = y
USE_OXC_SDFAT = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
REQUIRE_SPECIAL_CLOCK = y


SRCS  = main.cpp

SRCS += stm32f4_clock_HSE144.c

SRCS += adc_init_exa_4ch_dma_n.cpp

SRCS += f4_adcdma_n.cpp

SRCS += tim2_adcdma_init.cpp

SRCS += f4_sdio_init_1bit.cpp

SRCS += oxc_bsp_f429i_disc_sdram.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f4_modules.mk

SRCS += stm32f4xx_hal_adc_ex.c
SRCS += stm32f4xx_hal_tim_ex.c
SRCS += stm32f4xx_hal_dma_ex.c
SRCS += stm32f4xx_hal_sdram.c
SRCS += stm32f4xx_ll_fmc.c

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

