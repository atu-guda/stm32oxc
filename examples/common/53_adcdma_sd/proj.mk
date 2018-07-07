PROJ_NAME=adcdma_sd

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_CONSOLE_UART = y
# USE_OXC_CONSOLE_USB_CDC = y
USE_OXC_CONSOLE_DEFAULT = y
# USE_OXC_I2C = y
# USE_OXC_SPI = y
USE_OXC_TIM = y
USE_OXC_ADC = y
# USE_OXC_DAC = y
USE_OXC_DMA = y
USE_OXC_SD  = y
USE_OXC_SDFAT = y
USE_FREERTOS = y
# NOUSE_OXC_OSFUN = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
REQUIRE_SPECIAL_CLOCK = y

SRCS  = main.cpp

SRCS += adc_init_exa_4ch_dma.cpp
SRCS += tim2_adcdma_init.cpp


SRCS += f4_sdio_init_1bit.cpp

