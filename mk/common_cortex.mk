# Must be set before:
# MCTYPE like STM32F446
# may be
# BSPNAME
# MCINCTYPE like STM32F103xB (for HAL define), default: $(MCTYPE)xx
# BOARDNAME for STM BSP dirs

ifndef MCTYPE
  $(error MCTYPE not specified)
endif

ifndef MCINCTYPE
 MCINCTYPE := $(MCTYPE)xx
endif

# MCBASE is like "STM32F4"
MCBASE := $(shell echo "$(MCTYPE)" | head -c 7  )
# MCSUFF_U is like "F4"
MCSUFF_U := $(shell echo -n  '$(MCBASE)'| tail -c 2  )
# MCSUFF is like "f4" = ARCH
MCSUFF := $(shell m1='$(MCSUFF_U)'; echo  "$${m1,,*}" )
$(info PROJ_NAME= $(PROJ_NAME) BOARDNAME= $(BOARDNAME) BSPNAME= $(BSPNAME) )
$(info MCTYPE= $(MCTYPE)  MCBASE= $(MCBASE)  MCSUFF= $(MCSUFF) MCSUFF_U= $(MCSUFF_U) )

# OXCDIR := oxc // from Makefile TODO: from pkgconfig
OXCINC = $(OXCDIR)/inc
OXCSRC = $(OXCDIR)/src
OXCSRCARCH = $(OXCDIR)/src/arch/$(MCSUFF)
OXCINCARCH = $(OXCDIR)/inc/arch/$(MCSUFF)
OXCINCBSP  = $(OXCDIR)/inc/bsp/$(BSPNAME)
OXCBOARDDIR=$(OXCSRC)/bsp/$(BSPNAME)
STMBOARDDIR=$(STM32_HAL_FW_DIR)/Drivers/BSP/$(BOARDNAME)
STMCOMPONENTS=$(STM32_HAL_FW_DIR)/Drivers/BSP/Components
OXCLD = $(OXCDIR)/ld

ifndef FATFS_DIR
  FATFS_DIR := /usr/share/fatfs/source
endif

ifndef STM32_HAL_REPODIR
  ifdef OXC_USE_GLOBAL_REPO
    STM32_HAL_REPODIR = /usr/share/stm32cube/Repository
  else
    STM32_HAL_REPODIR = $(HOME)/STM32Cube/Repository
  endif
endif

ifndef STM32_HAL_FW
  ifdef OXC_USE_GLOBAL_REPO
    STM32_HAL_FW = $(MCSUFF)
  else
    STM32_HAL_FW = $(shell cd $(STM32_HAL_REPODIR) ; ls -1 -d STM32Cube_FW_$(MCSUFF_U)_V* | sort -V | head -1)
  endif
endif
STM32_HAL_FW_DIR = $(STM32_HAL_REPODIR)/$(STM32_HAL_FW)

$(info STM32_HAL_REPODIR= $(STM32_HAL_REPODIR)  STM32_HAL_FW_DIR= $(STM32_HAL_FW_DIR) )

ifeq ("$(wildcard $(STM32_HAL_FW_DIR)/package.xml)","")
  $(error Directory $(STM32_HAL_FW_DIR) is bad or nonexistent )
endif

STM_HAL_INC := \
 -I$(STM32_HAL_FW_DIR)/Drivers/STM32$(MCSUFF_U)xx_HAL_Driver/Inc \
 -I$(STM32_HAL_FW_DIR)/Drivers/STM32$(MCSUFF_U)xx_HAL_Driver/Inc/Legacy \
 -I$(STM32_HAL_FW_DIR)/Drivers/CMSIS/Device/ST/STM32$(MCSUFF_U)xx/Include \
 -I$(STM32_HAL_FW_DIR)/Drivers/CMSIS/Include

STM_HAL_SRC := \
 $(STM32_HAL_FW_DIR)/Drivers/STM32$(MCSUFF_U)xx_HAL_Driver/Src \
 $(STM32_HAL_FW_DIR)/Drivers/CMSIS/Device/ST/STM32$(MCSUFF_U)xx/Source/Templates \
 $(STM32_HAL_FW_DIR)/Drivers/CMSIS/Device/ST/STM32$(MCSUFF_U)xx/Source/Templates/gcc


# $(error Debug stop )

# common variables and rules to make stm32 binaries
TARGET:=arm-none-eabi
CC:=$(TARGET)-gcc
CXX:=$(TARGET)-g++
CPP:=$(TARGET)-cpp
AS:=$(TARGET)-gcc -x assembler-with-cpp
OBJCOPY:=$(TARGET)-objcopy
OBJDUMP:=$(TARGET)-objdump
LINK=$(CXX)



DEPSDIR=.deps
OBJDIR=.objs


# FreeRTOS: rtos/Source -> /usr/share/FreeRTOS/Source (or ../common/rtos ...)
RTDIR=/usr/share/FreeRTOS/Source
RTINC=$(RTDIR)/include

###################################################

ALLFLAGS += -g3 -O2
ALLFLAGS += -Wall -Wextra -Wundef -Wdouble-promotion
ALLFLAGS += -fno-common -ffunction-sections -fdata-sections
ALLFLAGS += -DSTM32 -D$(MCINCTYPE) -DHSE_VALUE=$(HSE_VALUE) -DUSE_HAL_LEGACY
CWARNFLAGS := -Wimplicit-function-declaration -Wmissing-prototypes -Wstrict-prototypes -Wno-unused-parameter -Wno-misleading-indentation
CXXWARNFLAGS := -Wno-unused-parameter -Wno-register

ALLFLAGS += -DPROJ_NAME=\"$(PROJ_NAME)\"
ALLFLAGS += -ffreestanding
ALLFLAGS += -mlittle-endian
# ALLFLAGS += --specs=nano.specs
# ALLFLAGS += -fstack-usage
ifeq "$(NO_STDLIB)" "y"
  ALLFLAGS += -nostdlib
endif

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
  ARCHFLAGS = -mthumb -mcpu=cortex-m3 -mfix-cortex-m3-ldrd -D__ARM_FEATURE_DSP=0
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
ifeq "$(MCBASE)" "STM32H7"
  ARCHFLAGS += -mthumb -mcpu=cortex-m7 -mfloat-abi=$(FLOAT_ABI) -mfpu=fpv5-d16
  KNOWN_MCU := yes
endif

ifneq "$(KNOWN_MCU)" "yes"
  $(warning Unknown MCU base $(MCBASE))
endif

ALLFLAGS += $(ARCHFLAGS)
ALLFLAGS += $(CFLAGS_ADD) $(ADDINC)

LDFLAGS += --static # -nostartfiles
LDFLAGS += -g3
LDFLAGS += -L$(OXCLD)
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,-Map=$(PROJ_NAME).map
LDFLAGS += -Wl,--gc-sections
LDFLAGS += $(ARCHFLAGS)
LDFLAGS += -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group

###################################################

SRCPATHS +=  $(STM_HAL_SRC) $(OXCBOARDDIR) $(ADDSRC)

ALLFLAGS += -I. $(STM_HAL_INC)

ifneq ($(origin BOARDNAME),undefined)
  ALLFLAGS += -I$(STMBOARDDIR)
  SRCPATHS +=   $(STMBOARDDIR)
endif


ifeq "$(NO_COMMON_HAL_MODULES)" "y"
  ALLFLAGS += -DNO_COMMON_HAL_MODULES
endif

# default: from src/bsp/$(BOARDNAME)/
SRCS += bsp_arch.cpp

ifeq "$(USE_OXC_CONSOLE_UART)" "y"
  # $(info "Used UART console" )
  USE_OXC_CONSOLE = y
  USE_OXC_UART = y
  SRCS += oxc_usartio.cpp
  ALLFLAGS += -DUSE_OXC_CONSOLE_UART
endif

ifeq "$(USE_OXC_UART)" "y"
  # $(info "Used UART" $(NOUSE_DEFAULT_UART_INIT) z )
  SRCS += stm32$(MCSUFF)xx_hal_uart.c
  ifneq "$(NOUSE_DEFAULT_UART_INIT)" "y"
    SRCS  += oxc_uart_default_init.cpp
  endif
  ALLFLAGS += -DUSE_OXC_UART
endif

ifeq "$(USE_OXC_CONSOLE_USB_CDC)" "y"
  $(info "Used USB_CDC console" )
  USE_USB_CDC  = y
  USE_OXC_CONSOLE = y
  ALLFLAGS += -DUSE_OXC_CONSOLE_USB_CDC
  SRCS += oxc_usbcdcio.cpp
endif

ifeq "$(USE_USB_CDC)" "y"
  $(info "Used USB_CDC" )
  ALLFLAGS += -DUSE_USB_CDC
  ALLFLAGS += -I$(OXCINCARCH)/usb_cdc -I$(OXCINC)/usbd_descr_cdc
  USE_USBD_LIB  = y
  STM_HAL_INC += -I$(STM32_HAL_FW_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc
  SRCPATHS += $(STM32_HAL_FW_DIR)/Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Src
  SRCPATHS += $(OXCSRCARCH)/usb_cdc
  SRCS += usbd_conf.cpp
  SRCS += usbd_desc.cpp
endif

ifeq "$(USE_USBD_LIB)" "y"
  $(info "Used USBD_LIB" )
  STM_HAL_INC += -I$(STM32_HAL_FW_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Inc
  SRCPATHS += $(STM32_HAL_FW_DIR)/Middlewares/ST/STM32_USB_Device_Library/Core/Src
  # USB: hal:
  SRCS += stm32$(MCSUFF)xx_hal_pcd.c
  SRCS += stm32$(MCSUFF)xx_hal_pcd_ex.c
  SRCS += stm32$(MCSUFF)xx_ll_usb.c
  # USB: lib:
  SRCS += usbd_core.c
  SRCS += usbd_cdc.c
  SRCS += usbd_ctlreq.c
  SRCS += usbd_ioreq.c
  ALLFLAGS += -DUSE_USBD_LIB
endif

ifeq "$(USE_OXC_MSCFAT)" "y"
  USE_USBH_MSC = y
  USE_OXC_FATFS = y
  ALLFLAGS += -DUSE_OXC_MSCFAT
endif

ifeq "$(USE_USBH_MSC)" "y"
  USE_USBH_LIB = y
  ALLFLAGS += -DUSE_USBH_MSC -I$(OXCINCARCH)/usbh_msc
  STM_HAL_INC += -I$(STM32_HAL_FW_DIR)/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Inc
  SRCPATHS += $(STM32_HAL_FW_DIR)/Middlewares/ST/STM32_USB_Host_Library/Class/MSC/Src
  SRCPATHS += $(OXCSRCARCH)/usbh_msc
  SRCS += usbh_msc.c
  SRCS += usbh_msc_bot.c
  SRCS += usbh_msc_scsi.c
  SRCS += usbh_conf.cpp
  SRCS += usbh_diskio.cpp
  # SRCS += 
endif

ifeq "$(USE_USBH_LIB)" "y"
  $(info "Used USBH_LIB" )
  STM_HAL_INC += -I$(STM32_HAL_FW_DIR)/Middlewares/ST/STM32_USB_Host_Library/Core/Inc
  SRCPATHS += $(STM32_HAL_FW_DIR)/Middlewares/ST/STM32_USB_Host_Library/Core/Src
  # # USB: hal:
  SRCS += stm32$(MCSUFF)xx_ll_usb.c
  SRCS += stm32$(MCSUFF)xx_hal_hcd.c
  # # USB: lib:
  SRCS += usbh_core.c
  SRCS += usbh_ctlreq.c
  SRCS += usbh_ioreq.c
  SRCS += usbh_pipes.c
  ALLFLAGS += -DUSE_USBH_LIB
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

ifeq "$(OXC_FAKE_IO)" "y"
  $(info "Used OXC_FAKE_IO" )
  ALLFLAGS += -DOXC_FAKE_IO
endif

ifeq "$(USE_OXC_DEVIO)" "y"
  # $(info "Used DEVIO" )
  USE_OXC = y
  SRCS += oxc_devio.cpp
  ALLFLAGS += -DUSE_OXC_DEVIO
endif

ifeq "$(USE_OXC_I2C)" "y"
  USE_OXC = y
  SRCS += oxc_i2c.cpp
  SRCS += stm32$(MCSUFF)xx_hal_i2c.c
  ifneq "$(MCSUFF)" "f1"
    SRCS += stm32$(MCSUFF)xx_hal_i2c_ex.c
  endif
  ifneq "$(NOUSE_DEFAULT_I2C_INIT)" "y"
    SRCS  += oxc_i2c_default_init.cpp
  endif
  ifeq "$(USE_OXC_DEBUG)" "y"
    SRCS += oxc_debug_i2c.cpp
  endif
  ALLFLAGS += -DUSE_OXC_I2C
endif

ifeq "$(USE_OXC_SPI)" "y"
  USE_OXC = y
  SRCS += oxc_spi.cpp
  SRCS += stm32$(MCSUFF)xx_hal_spi.c
  ifneq "$(NOUSE_DEFAULT_SPI_INIT)" "y"
    SRCS  += oxc_spi_init_default.cpp
  endif
  # ifeq "$(USE_OXC_DEBUG)" "y"
  #   SRCS += oxc_debug_spi.cpp
  # endif
  ALLFLAGS += -DUSE_OXC_SPI
endif

ifeq "$(USE_OXC_TIM)" "y"
  USE_OXC = y
  SRCS += oxc_tim.cpp
  SRCS += stm32$(MCSUFF)xx_hal_tim.c
  SRCS += stm32$(MCSUFF)xx_hal_tim_ex.c
  ALLFLAGS += -DUSE_OXC_TIM
endif


ifeq "$(USE_OXC_ADC)" "y"
  USE_OXC = y
  SRCS += stm32$(MCSUFF)xx_hal_adc.c
  SRCS += stm32$(MCSUFF)xx_hal_adc_ex.c
  SRCS += oxc_arch_adc.cpp
  SRCS += oxc_adc.cpp
  ALLFLAGS += -DUSE_OXC_ADC
endif

ifeq "$(USE_OXC_DAC)" "y"
  USE_OXC = y
  # SRCS += oxc_dac.cpp
  SRCS += stm32$(MCSUFF)xx_hal_dac.c
  SRCS += stm32$(MCSUFF)xx_hal_dac_ex.c
  ALLFLAGS += -DUSE_OXC_DAC
endif

ifeq "$(USE_OXC_DMA)" "y"
  USE_OXC = y
  # SRCS += oxc_dma.cpp
  SRCS += stm32$(MCSUFF)xx_hal_dma.c
  ifeq "$(MCSUFF)" "f4"
    SRCS += stm32$(MCSUFF)xx_hal_dma_ex.c
  endif
  ifeq "$(MCSUFF)" "f7"
    SRCS += stm32$(MCSUFF)xx_hal_dma_ex.c
  endif
  ifeq "$(MCSUFF)" "h7"
    SRCS += stm32$(MCSUFF)xx_hal_dma_ex.c
  endif
  ALLFLAGS += -DUSE_OXC_DMA
endif

ifeq "$(USE_OXC_SDFAT)" "y"
  USE_OXC = y
  USE_OXC_SD = y
  USE_OXC_FATFS = y
  SRCS += fatfs_sd_st.c
  SRCS += sd_diskio.c
  ALLFLAGS += -DUSE_OXC_SDFAT
endif

ifeq "$(USE_OXC_SD)" "y"
  USE_OXC = y
  SRCS += stm32$(MCSUFF)xx_hal_sd.c
  SRCS += stm32$(MCSUFF)xx_ll_sdmmc.c
  SRCS += bsp_driver_sd.c
  ALLFLAGS += -DUSE_OXC_SD
endif

ifeq "$(USE_OXC_FATFS)" "y"
  ADDSRC   +=   $(FATFS_DIR)
  ALLFLAGS += -I$(FATFS_DIR) -DUSE_OXC_FATFS
  SRCS += ff.c
  SRCS += ffunicode.c
  SRCS += ff_gen_drv_st.c
  SRCS += diskio_st.c
  SRCS += oxc_fs_cmd0.cpp
  ifeq "$(USE_FREERTOS)" "y"
    SRCS += oxc_ff_syncobj.cpp
  endif
endif

ifeq "$(USE_OXC_DEBUG)" "y"
  USE_OXC = y
  SRCS += oxc_debug1.cpp
  ALLFLAGS += -DUSE_OXC_DEBUG
endif

ifeq "$(USE_OXC_SDRAM)" "y"
  USE_OXC = y
  SRCS += bsp_sdram_msp.cpp
  SRCS += oxc_sdram_common.cpp
  SRCS += stm32$(MCSUFF)xx_hal_sdram.c
  SRCS += stm32$(MCSUFF)xx_ll_fmc.c
  ifeq "$(MCSUFF)" "h7"
    SRCS += stm32$(MCSUFF)xx_hal_mdma.c
  endif
  ALLFLAGS += -DUSE_OXC_SDRAM
endif


ifeq "$(USE_OXC)" "y"
  SRCPATHS += $(OXCSRC) $(OXCSRCARCH)
  ALLFLAGS += -DUSE_OXC -I$(OXCINC) -I$(OXCINCBSP) -I$(OXCINCARCH) -I$(OXCINC)/fake
  SRCS += oxc_base.cpp
  SRCS += oxc_miscfun.cpp
  SRCS += oxc_gpio.cpp
  ifneq "$(OXC_NO_OUTSTREAM)" "y"
    SRCS += oxc_outstream.cpp
  endif
  ifneq "$(OXC_NO_RINGBUF)" "y"
    SRCS += oxc_mutex.cpp
    SRCS += oxc_ringbuf.cpp
  endif
  ifneq "$(OXC_NO_OSFUN)" "y"
    SRCS += oxc_osfun.cpp
  else
    ALLFLAGS += -DOXC_NO_OSFUN
  endif
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
  ALLFLAGS += -I$(STM32_HAL_FW_DIR)/Utilities/Fonts
  SRCPATHS += $(STM32_HAL_FW_DIR)/Utilities/Fonts
  SRCS += font8.c
  SRCS += font12.c
  SRCS += font16.c
  SRCS += font20.c
  SRCS += font24.c
endif


vpath %.c   $(SRCPATHS)
vpath %.cpp $(SRCPATHS)
vpath %.s   $(STM32_HAL_FW_DIR)/Drivers/CMSIS/Device/ST/STM32$(MCSUFF_U)xx/Source/Templates/gcc $(OXCSRC)
vpath %.o   $(OBJDIR)
vpath %.d   $(DEPSDIR)
vpath %.ld  $(OXCLD)


OBJS0a = $(SRCS:.cpp=.o)
OBJS0 = $(OBJS0a:.c=.o)
OBJS  = $(OBJS0:.s=.o)
OBJS1 = $(addprefix $(OBJDIR)/,$(OBJS))

CFLAGS   = $(ALLFLAGS)  -std=c11   $(CWARNFLAGS)
CXXFLAGS = $(ALLFLAGS)  -std=c++17 $(CXXWARNFLAGS) -fno-rtti -fno-exceptions -fno-threadsafe-statics -fno-use-cxa-atexit

$(info SRCPATHS is $(SRCPATHS) )
$(info STM_HAL_INC= $(STM_HAL_INC) )

###################################################

.PHONY: proj flash clean subclean

all: proj dirs

dirs:
	mkdir -p $(DEPSDIR) $(OBJDIR)

proj:  dirs $(PROJ_NAME).bin

$(OBJDIR)/*.o:  $(MAKEFILE_LIST)


$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c -MMD -o $@ $<
	mv $(OBJDIR)/$*.d $(DEPSDIR)

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -MMD -o $@ $<
	mv $(OBJDIR)/$*.d $(DEPSDIR)

$(OBJDIR)/%.o: %.s
	$(AS) $(CFLAGS) -I$(OXCSRC)/startup -c -o $@ $<

$(PROJ_NAME).elf: $(OBJS1) $(LDSCRIPT)
	$(LINK) $(OBJS1) $(LDFLAGS) $(LIBS) -o $(PROJ_NAME).elf
	$(OBJDUMP) -h -f -d -S $(PROJ_NAME).elf > $(PROJ_NAME).lst

$(PROJ_NAME).bin: $(PROJ_NAME).elf
	$(OBJCOPY) -O binary $(PROJ_NAME).elf $(PROJ_NAME).bin

#	$(OBJCOPY) -O ihex $(PROJ_NAME).elf $(PROJ_NAME).hex

#flash: $(PROJ_NAME).bin
#	st-flash --reset write  $(PROJ_NAME).bin 0x8000000

flash: $(PROJ_NAME).elf
	eblink-stm32-flash $<


subclean:
	rm -f *.o *.d $(OBJDIR)/*.o $(DEPSDIR)/*.d
	rm -f $(PROJ_NAME).elf
	rm -f $(PROJ_NAME).hex
	rm -f $(PROJ_NAME).lst
	rm -f $(PROJ_NAME).map

clean: subclean
	rm -f $(PROJ_NAME).bin


#

