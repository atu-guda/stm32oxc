PROJ_NAME=i2c_ds3231

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_I2C = y
# USE_OXC_SPI = y
# USE_OXC_TIM = y
# USE_OXC_ADC = y
# USE_OXC_DAC = y
# USE_OXC_DMA = y
# USE_OXC_SD  = y
# USE_OXC_SDFAT = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
# REQUIRE_SPECIAL_CLOCK = y


SRCS  = main.cpp

SRCS += oxc_ds3231.cpp

