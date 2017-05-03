PROJ_NAME=stemwin_test

ADDINC = -I$(STMINC)/STemWin -I$(STMCOMPONENTS)/ili9341 -I$(STMSRC)
ADDSRC =  $(STMSRC)/fonts $(STMCOMPONENTS)/stmpe811 $(STMCOMPONENTS)/ili9341 $(STMSRC)/STemWin/OS

# REQUIRE_SPECIAL_CLOCK = y

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
# SRCS += BASIC_HelloWorld.c
SRCS += GUIConf.c
SRCS += LCDConf_stm32f429i_disco_MB1075.c

SRCS += uart1_init_a9a10.cpp
SRCS += oxc_bsp_f429i_disc_sdram.cpp

# SRCS += empty_test0_function.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# list: see ../common/f4_modules.mk

SRCS += stm32f4xx_hal_dma.c
SRCS += stm32f4xx_hal_spi.c
SRCS += stm32f4xx_hal_i2c.c
SRCS += stm32f4xx_hal_sdram.c
SRCS += stm32f4xx_hal_ltdc.c
SRCS += stm32f4xx_hal_dma2d.c
SRCS += stm32f4xx_hal_rcc_ex.c
SRCS += stm32f4xx_ll_fmc.c

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

LIBS += $(STMDIR)/lib/STemWin/STemWin532_CM4_GCC.a


include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

