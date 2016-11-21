PROJ_NAME=adc0

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_USART = n
USE_OXC_CONSOLE_USB_CDC = y
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
USE_USB_DEFAULT_CDC = y


# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc

SRCS  = main.cpp
SRCS += adc_init.cpp

#SRCS += oxc_i2c.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# list: see ../common/f4_modules.mk

SRCS += stm32f4xx_hal_adc.c

# USB modules added automaticaly  if USE_OXC_CONSOLE_USB_CDC = y

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

