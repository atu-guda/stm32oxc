OXCDIR = /usr/share/stm32oxc

COMMONPROJDIR = $(shell basename `pwd` )
include ../../common/$(COMMONPROJDIR)/proj.mk


# aux: (hal, cortex, gpio, rcc added by bsp makefile part )
# see ../common/f4_modules.mk

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

SRCS += stm32f446_clock_HSE144.c

include $(wildcard $(DEPSDIR)/*.d)

Makefile: ../../common/$(COMMONPROJDIR)/proj.mk

#

