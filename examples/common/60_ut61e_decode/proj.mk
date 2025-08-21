PROJ_NAME=ut61e_decode

USE_OXC = y
USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
# USE_OXC_CONSOLE_UART = y
USE_OXC_UART_EX = y
# USE_OXC_CONSOLE_USB_CDC = y
USE_OXC_CONSOLE_DEFAULT = y
# USE_OXC_I2C = y
# USE_OXC_SPI = y
# USE_OXC_TIM = y
# USE_OXC_ADC = y
# USE_OXC_DAC = y
USE_OXC_DMA = y
# USE_OXC_SD  = y
# USE_OXC_SDFAT = y
# USE_FREERTOS = y
# NOUSE_OXC_OSFUN = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
# REQUIRE_SPECIAL_CLOCK = y

# NOUSE_DEFAULT_UART_INIT = y

SRCS  = main.cpp

SRCS += oxc_floatfun.cpp

SRCS += oxc_ut61e_decode.cpp
SRCS += uart_for_ut61e.cpp

