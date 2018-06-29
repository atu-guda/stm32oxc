# common variables and rules to make stm32 binaries
TARGET:=arm-none-eabi
CC:=$(TARGET)-gcc
CXX:=$(TARGET)-g++
CPP:=$(TARGET)-cpp
OBJCOPY:=$(TARGET)-objcopy
OBJDUMP:=$(TARGET)-objdump
LINK=$(CXX)

STMDIR=/usr/share/stm32cube
STMINC=$(STMDIR)/inc
STMSRC=$(STMDIR)/src
STMLD=$(STMDIR)/ld
STMMK=$(STMDIR)/mk
STMBSP=$(STMDIR)/bsp
STMBOARDDIR=$(STMBSP)/$(BOARDNAME)
STMCOMPONENTS=$(STMBSP)/Components

# OXCDIR := oxc // from Makefile
OXCINC = $(OXCDIR)/inc
OXCSRC = $(OXCDIR)/src

DEPSDIR=.deps
OBJDIR=.objs


# FreeRTOS: rtos/Source -> /usr/share/FreeRTOS/Source (or ../common/rtos ...)
RTDIR=/usr/share/FreeRTOS/Source
RTINC=$(RTDIR)/include

###################################################

ALLFLAGS  = -g3 -O2
ALLFLAGS += -Wall -Wextra -Wundef
ALLFLAGS += -fno-common -ffunction-sections -fdata-sections
ALLFLAGS += -D$(MCINCTYPE) -DHSE_VALUE=$(HSE_VALUE) -DUSE_HAL_LEGACY
CWARNFLAGS := -Wimplicit-function-declaration -Wmissing-prototypes -Wstrict-prototypes -Wno-unused-parameter
CXXWARNFLAGS := -Wno-unused-parameter -Wno-register

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


ifeq "$(USE_FLOAT_SOFTFP)" "y"
  FLOAT_ABI = softfp
else
  FLOAT_ABI = hard
endif

KNOWN_MCU := no
ifeq "$(MCBASE)" "STM32F0"
  ARCHFLAGS = -mthumb -mcpu=cortex-m0 -mfix-cortex-m3-ldrd
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32L0"
  ARCHFLAGS = -mthumb -mcpu=cortex-m0 -mfix-cortex-m3-ldrd
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F1"
  ARCHFLAGS = -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32L1"
  ARCHFLAGS = -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F2"
  ARCHFLAGS = -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F3"
  ARCHFLAGS = -mthumb -mcpu=cortex-m4 -mfloat-abi=$(FLOAT_ABI) -mfpu=fpv4-sp-d16
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F4"
  ARCHFLAGS += -mthumb -mcpu=cortex-m4 -mfloat-abi=$(FLOAT_ABI) -mfpu=fpv4-sp-d16
  KNOWN_MCU := yes
endif
ifeq "$(MCBASE)" "STM32F7"
  ARCHFLAGS += -mthumb -mcpu=cortex-m7 -mfloat-abi=$(FLOAT_ABI) -mfpu=fpv4-sp-d16
  KNOWN_MCU := yes
endif

ifneq "$(KNOWN_MCU)" "yes"
  $(warning Unknown MCU base $(MCBASE))
endif

ALLFLAGS += $(ARCHFLAGS)
ALLFLAGS += $(CFLAGS_ADD) $(ADDINC)

LDFLAGS  = --static # -nostartfiles
LDFLAGS += -g3
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map=$(PROJ_NAME).map
LDFLAGS += -Wl,--gc-sections
LDFLAGS += $(ARCHFLAGS)
LDFLAGS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

###################################################

ALLFLAGS += -I. -I$(STMINC) -I$(STMBOARDDIR)

SRCPATHS =  $(STMSRC) $(STMSRC)/templates  $(STMBOARDDIR) $(ADDSRC)

ifeq "$(USE_OXC_CONSOLE_USART)" "y"
  # $(info "Used USART console" )
  USE_OXC_CONSOLE = y
  SRCS += oxc_usartio.cpp
  SRCS += stm32$(MCSUFF)xx_hal_uart.c
  ifneq "$(NOUSE_DEFAULT_UART_INIT)" "y"
    SRCS  += oxc_uart_default_init.cpp
  endif
  ALLFLAGS += -DUSE_OXC_CONSOLE_USART
endif

ifeq "$(USE_OXC_CONSOLE_USB_CDC)" "y"
  # $(info "Used USB_CDC console" )
  USE_USB = y
  USE_OXC_CONSOLE = y
  SRCS += oxc_usbcdcio.cpp
  SRCS += usbfs_init.cpp
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
  SRCS += oxc_ministr.cpp
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
  ifneq "$(NOUSE_OXC_OSFUN)" "y"
    SRCS += oxc_osfun.cpp
  endif
endif

ifeq "$(USE_OXC_I2C)" "y"
  SRCS += oxc_i2c.cpp
  SRCS += stm32$(MCSUFF)xx_hal_i2c.c
  ifneq "$(NOUSE_DEFAULT_I2C_INIT)" "y"
    SRCS  += oxc_i2c_default_init.cpp
  endif
  ifeq "$(USE_OXC_DEBUG)" "y"
    SRCS += oxc_debug_i2c.cpp
  endif
  # f3 special case
  ifeq "$(MCSUFF)" "f3"
    SRCS += stm32$(MCSUFF)xx_hal_i2c_ex.c
  endif
  # f7 special case
  ifeq "$(MCSUFF)" "f7"
    SRCS += stm32$(MCSUFF)xx_hal_i2c_ex.c
  endif
endif

ifeq "$(USE_OXC_SPI)" "y"
  SRCS += oxc_spi.cpp
  SRCS += stm32$(MCSUFF)xx_hal_spi.c
  ifneq "$(NOUSE_DEFAULT_SPI_INIT)" "y"
    SRCS  += oxc_spi_init_default.cpp
  endif
  # ifeq "$(USE_OXC_DEBUG)" "y"
  #   SRCS += oxc_debug_spi.cpp
  # endif
endif


ifeq "$(USE_FREERTOS)" "y"
  RTARCH = $(RTDIR)/portable/GCC/$(FREERTOS_ARCHNAME)
  SRCPATHS += $(RTDIR) $(RTARCH) $(RTDIR)/portable/MemMang
  ALLFLAGS += -I$(RTINC)  -I$(RTARCH) -DUSE_FREERTOS
  ifndef FREERTOS_HEAP
    FREERTOS_HEAP = heap_3.c
  endif
  # SRCS += croutine.c
  # SRCS += event_groups.c
  SRCS += list.c
  SRCS += queue.c
  SRCS += tasks.c
  SRCS += timers.c
  SRCS += $(FREERTOS_HEAP)
  SRCS += port.c
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
  # USB: hal:
  SRCS += stm32$(MCSUFF)xx_hal_pcd.c
  SRCS += stm32$(MCSUFF)xx_hal_pcd_ex.c
  ifneq "$(MCSUFF)" "f3"
    SRCS += stm32$(MCSUFF)xx_ll_usb.c
  endif
  # USB: lib:
  SRCS += usbd_core.c
  SRCS += usbd_cdc.c
  SRCS += usbd_ctlreq.c
  SRCS += usbd_ioreq.c
endif

ifeq "$(USE_DEFAULT_SDIO_FAT)" "y"
  SRCS += bsp_driver_sd.c
  SRCS += fatfs.c
  SRCS += ff.c
  SRCS += ff_gen_drv.c
  SRCS += diskio.c
  SRCS += sd_diskio.c
  SRCS += oxc_ff_syncobj.cpp
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

$(info SRCPATHS is $(SRCPATHS) )

###################################################

.PHONY: proj flash clean subclean

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



subclean:
	rm -f *.o *.d $(OBJDIR)/*.o $(DEPSDIR)/*.d
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).lst
	rm -f $(PROJ_NAME).map

clean: subclean
	rm -f $(PROJ_NAME).bin


#

