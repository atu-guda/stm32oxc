PROJ_NAME=tcoup


USE_OXC = y
USE_OXC_UART = y
USE_OXC_DEVIO = y
# USE_OXC_DEBUG = y
USE_OXC_CONSOLE_UART = n
# USE_OXC_CONSOLE_USB_CDC = y
# USE_OXC_CONSOLE_DEFAULT = y
# USE_OXC_I2C = y
# USE_OXC_SPI = y
# USE_OXC_TIM = y
# USE_OXC_ADC = y
# USE_OXC_DAC = y
USE_OXC_DMA = y
# USE_OXC_SD  = y
# USE_OXC_SDFAT = y
USE_FREERTOS = y
# NOUSE_OXC_OSFUN = y
OXC_FAKE_IO = y
# FREERTOS_HEAP = heap_2.c # # default: heap_3.c
# REQUIRE_SPECIAL_CLOCK = y

NOUSE_DEFAULT_UART_INIT = y


OXCDIR = /usr/share/stm32oxc


SRCS  = main.cpp

# for now
NOUSE_DEFAULT_UART_INIT = y

SRCS += stm32f1xx_hal_msp.cpp
SRCS += stm32f1xx_hal_spi.c

SRCS += oxc_common1.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )

#

