OXCDIR = /usr/share/stm32oxc
PROJ_NAME=stemwin_test

ADDINC = -I$(STMINC)/STemWin -I$(STMCOMPONENTS)/ili9341 -I$(STMSRC) -I$(STMINC)/fonts
ADDSRC =  $(STMSRC)/fonts $(STMCOMPONENTS)/stmpe811 $(STMCOMPONENTS)/ili9341  $(STMSRC)/STemWin $(STMSRC)/STemWin/OS


USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_CONSOLE_UART = y
# USE_OXC_CONSOLE_USB_CDC = y
USE_OXC_CONSOLE_DEFAULT = y
USE_OXC_I2C = y
# ------------------------- manual!
# USE_OXC_SPI = y
# USE_OXC_TIM = y
# USE_OXC_ADC = y
# USE_OXC_DAC = y
USE_OXC_DMA = y
# USE_OXC_SD  = y
# USE_OXC_SDFAT = y
USE_OXC_SDRAM = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
# REQUIRE_SPECIAL_CLOCK = y


SRCS  = main.cpp

SRCS += GUIConf.c
SRCS += LCDConf_stm32f429i_disco_MB1075.cpp

SRCS += stm32f4xx_hal_spi.c

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f4_modules.mk

SRCS += stm32f4xx_hal_ltdc.c
SRCS += stm32f4xx_hal_dma2d.c
SRCS += stm32f4xx_hal_rcc_ex.c

# from BSP
#SRCS += stm32f429i_discovery_sdram.c
SRCS += stmpe811.c
SRCS += ili9341.c
SRCS += stm32f429i_discovery.c
SRCS += stm32f429i_discovery_lcd.c
SRCS += stm32f429i_discovery_io.c
SRCS += stm32f429i_discovery_ts.c

# STemWin
SRCS += GUI_X.c

# TODO: version auto
LIBS += $(STMDIR)/lib/STemWin/STemWin540_CM4_GCC.a


include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

