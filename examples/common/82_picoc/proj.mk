PROJ_NAME=picoc

USE_OXC = y
USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_CONSOLE_UART = y
# USE_OXC_CONSOLE_USB_CDC = y
USE_OXC_CONSOLE_DEFAULT = y
# USE_OXC_I2C = y
# USE_OXC_SPI = y
# USE_OXC_TIM = y
# USE_OXC_ADC = y
# USE_OXC_DAC = y
# USE_OXC_DMA = y
# USE_OXC_SD  = y
# USE_OXC_SDFAT = y
# USE_FREERTOS = y
# NOUSE_OXC_OSFUN = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
# REQUIRE_SPECIAL_CLOCK = y

# NOUSE_DEFAULT_UART_INIT = y

# TODO: make good place
PICOC_DIR = /home/atu/proj/misc/picoc

SRCPATHS += $(PICOC_DIR)
ALLFLAGS += -I$(PICOC_DIR)

SRCS  = main.cpp

SRCS += clibrary.c
SRCS += debug.c
SRCS += expression.c
SRCS += heap.c
SRCS += include.c
SRCS += lex.c
SRCS += parse.c
SRCS += platform.c
SRCS += table.c
SRCS += type.c
SRCS += variable.c

SRCS += library_stm32oxc.c
SRCS += platform_stm32oxc.c
SRCS += oxc_picoc_math.c



