PROJ_NAME=adcdma_sd

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_I2C = y
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

SRCS += adc_init_exa_4ch_dma.cpp
SRCS += tim2_adcdma_init.cpp

# SRCS += stm32f446_clock_HSE144.c

SRCS += f4_sdio_init_1bit.cpp

