OXCDIR = /usr/share/stm32oxc

COMMONPROJDIR = $(shell basename `pwd` )
PROJMK=../../common/$(COMMONPROJDIR)/proj.mk
-include $(PROJMK)
-include local.mk

# aux: (hal, cortex, gpio, rcc added by bsp makefile part )
# see ../common/fX_modules.mk

include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)

ifeq ($(PROJMK),$(wildcard $(PROJMK)))
Makefile: ../../common/$(COMMONPROJDIR)/proj.mk
endif

#

