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

# OXCDIR := oxc // from Makefile
OXCINC = $(OXCDIR)/inc
OXCSRC = $(OXCDIR)/src

DEPSDIR=.deps
OBJDIR=.objs


# FreeRTOS: rtos/Source -> /usr/share/FreeRTOS/Source (or ../common/rtos ...)
RTDIR=rtos
RTSRC=$(RTDIR)/Source
RTINC=$(RTSRC)/include

###################################################

ALLFLAGS  = -g3 -O2
ALLFLAGS += -Wall -Wextra -Wundef
ALLFLAGS += -fno-common -ffunction-sections -fdata-sections
ALLFLAGS += -D$(MCINCTYPE) -DHSE_VALUE=$(HSE_VALUE)
CWARNFLAGS := -Wimplicit-function-declaration -Wmissing-prototypes -Wstrict-prototypes -Wno-unused-parameter
CXXWARNFLAGS := -Wno-unused-parameter

ALLFLAGS += -DPROJ_NAME=\"$(PROJ_NAME)\"
ALLFLAGS += -ffreestanding
ALLFLAGS += -mlittle-endian
ifeq "$(NO_STDLIB)" "y"
  ALLFLAGS += -nostdlib
endif

$(info MCTYPE is $(MCTYPE) )
# MCBASE is like "STM32F4"
MCBASE := $(shell echo "$(MCTYPE)" | head -c 7  )
$(info MCBASE is $(MCBASE) )
# MCSUFF is like "f4"
MCSUFF := $(shell m1='$(MCBASE)'; echo -n "$${m1,,*}" | tail -c 2  )
$(info MCSUFF is $(MCSUFF) )

ALLFLAGS  += -D$(MCTYPE) -D$(MCBASE) -DMCTYPE=$(MCTYPE) -DMCBASE=$(MCBASE)

KNOWN_MCU := no
ifeq "$(MCBASE)" "STM32F0"
  ARCHFLAGS = -mthumb -mcpu=cortex-m0 -mfix-cortex-m3-ldrd
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F1"
  ARCHFLAGS = -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F2"
  ARCHFLAGS = -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F3"
  ARCHFLAGS = -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F4"
  ARCHFLAGS += -mthumb -mcpu=cortex-m4 -mfloat-abi=softfp -mfpu=fpv4-sp-d16
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F7"
  ARCHFLAGS += -mthumb -mcpu=cortex-m7 -mfloat-abi=softfp -mfpu=fpv4-sp-d16
  KNOWN_MCU := yes
endif

ifneq "$(KNOWN_MCU)" "yes"
  $(warning Unknown MCU base $(MCBASE))
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

SRCPATHS =  $(STMSRC) $(STMSRC)/templates $(ADDSRC)

ifeq "$(USE_OXC_CONSOLE_USART)" "y"
  # $(info "Used USART console" )
  USE_OXC_CONSOLE = y
  SRCS += oxc_usartio.cpp
  ALLFLAGS += -DUSE_OXC_CONSOLE_USART
endif

ifeq "$(USE_OXC_CONSOLE_USB_CDC)" "y"
  # $(info "Used USB_CDC console" )
  USE_USB = y
  USE_OXC_CONSOLE = y
  SRCS += oxc_usbcdcio.cpp
  ALLFLAGS += -DUSE_OXC_CONSOLE_USB_CDC
endif

ifeq "$(USE_OXC_CONSOLE)" "y"
  # $(info "Used console" )
  USE_OXC_DEVIO = y
  SRCS += oxc_console.cpp
  SRCS += oxc_smallrl.cpp
  SRCS += oxc_common1.cpp
  ALLFLAGS += -DUSE_OXC_CONSOLE
else
  # $(info "NOT Used console" )
  ifeq "$(USE_OXC_DEBUG)" "y"
    $(warning "Console must be used if debug is in use")
  endif
endif

ifeq "$(USE_OXC_DEVIO)" "y"
  # $(info "Used DEVIO" )
  USE_OXC = y
  SRCS += oxc_devio.cpp
  SRCS += oxc_rtosqueue.cpp
  ALLFLAGS += -DUSE_OXC_DEVIO
  ifneq "$(USE_FREERTOS)" "y"
    $(warning "FreeRTOS must be used if DevIO is in use")
  endif
endif

ifeq "$(USE_OXC_DEBUG)" "y"
  SRCS += oxc_debug1.cpp
  ALLFLAGS += -DUSE_OXC_DEBUG
endif


ifeq "$(USE_OXC)" "y"
  SRCPATHS += $(OXCSRC)
  ALLFLAGS += -I$(OXCINC)
  SRCS += oxc_base.cpp
  SRCS += oxc_miscfun.cpp
  SRCS += oxc_gpio.cpp
  SRCS += oxc_osfun.cpp
endif

ifeq "$(USE_FREERTOS)" "y"
  SRCPATHS += $(RTSRC) $(RTDIR)
  ALLFLAGS += -I$(RTINC) -DUSE_FREERTOS
  ifndef FREERTOS_HEAP
    FREERTOS_HEAP = heap_3.c
  endif
  # rtos -> /usr/share/FreeRTOS
  #SRCS += croutine.c
  SRCS += list.c
  SRCS += queue.c
  SRCS += tasks.c
  SRCS += timers.c
  # symlink to Source/portable/GCC/ARM_CM3/port.c <<< change
  SRCS += $(FREERTOS_HEAP)
  # symlink to Source/portable/GCC/ARM_CM4F/port.c <<< change
  SRCS += port.c
  # Beware: portmacro.h is symlink to
  #  rtos/Source/portable/GCC/ARM_CM3/portmacro.h or CM4F
endif

ifeq "$(USE_FONTS)" "y"
  SRCPATHS += $(STMSRC)/fonts
  SRCS += font8.c
  SRCS += font12.c
  SRCS += font16.c
  SRCS += font20.c
  SRCS += font24.c
endif


ifeq "$(USE_USB_DEFAULT_CDC)" "y"
  SRCPATHS += $(OXCSRC)/usb_cdc_$(MCSUFF)
  ALLFLAGS += -I$(OXCINC)/usb_cdc_$(MCSUFF)
  SRCS += usbd_conf.cpp
  SRCS += usbd_desc.cpp
  # USB: lib:
  SRCS += usbd_core.c
  SRCS += usbd_cdc.c
  SRCS += usbd_ctlreq.c
  SRCS += usbd_ioreq.c
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

CFLAGS   = $(ALLFLAGS)  -std=c11   $(CWARNFLAGS)
CXXFLAGS = $(ALLFLAGS)  -std=c++11 $(CXXWARNFLAGS) -fno-rtti -fno-exceptions -fno-threadsafe-statics

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
	$(LINK) $^ $(LDFLAGS) $(LIBS) -o $@
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

