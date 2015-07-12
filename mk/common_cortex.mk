# common variables and rules to make stm32 binaries
TARGET:=arm-none-eabi
CC:=$(TARGET)-gcc
CXX:=$(TARGET)-g++
CPP:=$(TARGET)-cpp
OBJCOPY:=$(TARGET)-objcopy
OBJDUMP:=$(TARGET)-objdump

STMDIR=/usr/share/stm32cube
STMINC=$(STMDIR)/inc
STMSRC=$(STMDIR)/src
STMLD=$(STMDIR)/ld

# OXCDIR := ox // from Makefile
OXCINC = $(OXCDIR)/inc
OXCSRC = $(OXCDIR)/src

DEPSDIR=.deps
OBJDIR=.objs


# FreeRTOS: rtos/Source -> /usr/share/FreeRTOS/Source (or ../common/rtos ...)
RTDIR=rtos
RTSRC=$(RTDIR)/Source
RTINC=$(RTSRC)/include

###################################################

ALLFLAGS := -g3 -O2
ALLFLAGS += -Wall -Wextra -Wundef
ALLFLAGS += -fno-common -ffunction-sections -fdata-sections
CWARNFLAGS := -Wimplicit-function-declaration -Wmissing-prototypes -Wstrict-prototypes

#ALLFLAGS += -DUSE_STDPERIPH_DRIVER=1
ALLFLAGS += -DPROJ_NAME=\"$(PROJ_NAME)\"
ALLFLAGS += -ffreestanding
ALLFLAGS += -mlittle-endian
ifeq "$(NO_STDLIB)" "y"
  ALLFLAGS += -nostdlib
endif

$(info MCTYPE is $(MCTYPE) )
MCBASE := $(shell echo "$(MCTYPE)" | head -c 7  )
$(info MCBASE is $(MCBASE) )

ALLFLAGS  += -D$(MCTYPE) -D$(MCBASE) -DMCTYPE=$(MCTYPE) -DMCBASE=$(MCBASE)

ifeq "$(MCBASE)" "STM32F0"
  ARCHFLAGS = -mthumb -mcpu=cortex-m0 -mfix-cortex-m3-ldrd
endif
ifeq "$(MCBASE)" "STM32F1"
  ARCHFLAGS = -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd
endif
ifeq "$(MCBASE)" "STM32F2"
  ARCHFLAGS = -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd
endif
ifeq "$(MCBASE)" "STM32F3"
  ARCHFLAGS = -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16
endif
ifeq "$(MCBASE)" "STM32F4"
  ARCHFLAGS += -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16
endif


ALLFLAGS += $(ARCHFLAGS)
ALLFLAGS += $(CFLAGS_ADD)

LDFLAGS  = --static # -nostartfiles
LDFLAGS += -g3
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map=$(PROJ_NAME).map
LDFLAGS += -Wl,--gc-sections
LDFLAGS += $(ARCHFLAGS)
LDFLAGS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

###################################################

ALLFLAGS += -I. -I$(STMINC)

SRCPATHS =  $(STMSRC) $(STMSRC)/templates

ifeq "$(USE_OXC)" "y"
  SRCPATHS += $(OXCSRC)
  ALLFLAGS += -I$(OXCINC)
endif

ifeq "$(USE_FREERTOS)" "y"
  SRCPATHS += $(RTSRC) $(RTDIR)
  ALLFLAGS += -I$(RTINC) -DUSE_FREERTOS
endif

ifeq "$(USE_FONTS)" "y"
  SRCPATHS += $(OXCSRC)/fonts
endif

ifeq "$(USE_USB_OTG)" "y"
  # SRCPATHS += $(STMSRC)/usb_otg
  # ALLFLAGS += -I$(STMINC)/usb_otg
  # ALLFLAGS += -DUSE_USB_OTG_FS=1 -DUSE_EMBEDDED_PHY=1
endif

ifeq "$(USE_USB_DEFAULT_CDC)" "y"
  SRCPATHS += $(OXCSRC)/usb_cdc
  # ALLFLAGS += -I$(OXCINC)/usb_cdc
  SRCS += usbd_conf.cpp
  SRCS += usbd_desc.cpp
  # SRCS += usbd_cdc_vcp.cpp
  # SRCS += usbd_usr.cpp
  # # USB: lib:
  # SRCS += usb_dcd.c
  # SRCS += usb_core.c
  # SRCS += usbd_core.c
  # SRCS += usbd_req.c
  # SRCS += usbd_ioreq.c
  # SRCS += usbd_cdc_core.c
  # SRCS += usb_dcd_int.c
endif

vpath %.c   $(SRCPATHS) $(OXCSRC)/startup
vpath %.cpp $(SRCPATHS)
vpath %.s   $(OXCSRC)/startup $(STMSRC)/startup
vpath %.o   $(OBJDIR)
vpath %.d   $(DEPSDIR)


OBJS0a = $(SRCS:.cpp=.o)
OBJS0 = $(OBJS0a:.c=.o)
OBJS  = $(OBJS0:.s=.o)
OBJS1 = $(addprefix $(OBJDIR)/,$(OBJS))

CFLAGS   = $(ALLFLAGS)  -std=c11 $(CWARNFLAGS)
CXXFLAGS = $(ALLFLAGS)  -std=c++11 -fno-rtti -fno-exceptions -fno-threadsafe-statics

###################################################

.PHONY: proj

all: proj dirs

dirs:
	mkdir -p $(DEPSDIR) $(OBJDIR)

proj:  dirs $(PROJ_NAME).elf

$(OBJDIR)/*.o:  Makefile $(OXCDIR)/mk/common_cortex.mk


$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -o $@ $<
	mv $(OBJDIR)/$*.d $(DEPSDIR)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<
	mv $(OBJDIR)/$*.d $(DEPSDIR)

$(OBJDIR)/%.o: %.s
	$(CC) $(CFLAGS) -I$(OXCSRC)/startup -c -o $@ $<


$(PROJ_NAME).elf: $(OBJS1)
	$(LINK) $^ $(LDFLAGS) -o $@
	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin
	$(OBJDUMP) -h -f -d -S $(PROJ_NAME).elf > $(PROJ_NAME).lst

flash: $(PROJ_NAME).bin
	st-flash --reset write  $(PROJ_NAME).bin 0x8000000



clean:
	rm -f *.o *.d $(OBJDIR)/*.o $(DEPSDIR)/*.d
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).lst
	rm -f $(PROJ_NAME).map
	rm -f $(PROJ_NAME).bin

#

