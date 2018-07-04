OXCDIR = /usr/share/stm32oxc

include ../../common/53_adcdma_sd/proj.mk

# OXC_NO_STD_CONSOLE = y


SRCS  = main.cpp

SRCS += adc_init_exa_4ch_dma.cpp
SRCS += tim2_adcdma_init.cpp

SRCS += stm32f446_clock_HSE144.c

SRCS += f4_sdio_init_1bit.cpp

# aux: (hal, cortex, gpio, rcc added by ../common/board_add.mk )
# see ../common/f4_modules.mk


include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

