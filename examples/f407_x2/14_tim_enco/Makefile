PROJ_NAME=tim1_enco
MCTYPE = STM32F407

MCINCTYPE = $(MCTYPE)xx

LINK=$(CXX)

USE_OXC = y
USE_FREERTOS = y
NO_STDLIB = y
USE_USB_DEFAULT_CDC = y

# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc

#CFLAGS_ADD := -DSTM32F407xx -DHSE_VALUE=8000000
CFLAGS_ADD := -D$(MCINCTYPE) -DHSE_VALUE=8000000
CFLAGS_ADD += -Wno-unused-parameter
#
LDSCRIPT = $(STMLD)/STM32F407VE_FLASH.ld

SRCS  = main.cpp
SRCS += stm32f4xx_hal_msp.cpp
SRCS += system_stm32f4xx.c
SRCS += startup_stm32f407xx.s

SRCS += stm32f4_clock_HSE168.c

SRCS += oxc_base.cpp
SRCS += oxc_gpio.cpp
SRCS += oxc_devio.cpp
SRCS += oxc_rtosqueue.cpp
SRCS += oxc_osfun.cpp
SRCS += oxc_console.cpp
SRCS += oxc_debug1.cpp
#SRCS += oxc_usartio.cpp
SRCS += oxc_usbcdcio.cpp
SRCS += oxc_smallrl.cpp
#SRCS += oxc_smallrl_q.cpp
SRCS += oxc_common1.cpp
#SRCS += oxc_i2c.cpp


# stm32cube sources.
# Base:
SRCS += stm32f4xx_hal.c
SRCS += stm32f4xx_hal_cortex.c
SRCS += stm32f4xx_hal_gpio.c
SRCS += stm32f4xx_hal_rcc.c

# aux:
# SRCS += stm32f4xx_hal_adc.c
# SRCS += stm32f4xx_hal_adc_ex.c
# SRCS += stm32f4xx_hal_can.c
# SRCS += stm32f4xx_hal_cec.c
# SRCS += stm32f4xx_hal_crc.c
# SRCS += stm32f4xx_hal_cryp.c
# SRCS += stm32f4xx_hal_cryp_ex.c
# SRCS += stm32f4xx_hal_dac.c
# SRCS += stm32f4xx_hal_dac_ex.c
# SRCS += stm32f4xx_hal_dcmi.c
# SRCS += stm32f4xx_hal_dcmi_ex.c
# SRCS += stm32f4xx_hal_dma2d.c
# SRCS += stm32f4xx_hal_dma.c
# SRCS += stm32f4xx_hal_dma_ex.c
# SRCS += stm32f4xx_hal_eth.c
# SRCS += stm32f4xx_hal_flash.c
# SRCS += stm32f4xx_hal_flash_ex.c
# SRCS += stm32f4xx_hal_flash_ramfunc.c
# SRCS += stm32f4xx_hal_fmpi2c.c
# SRCS += stm32f4xx_hal_fmpi2c_ex.c
# SRCS += stm32f4xx_hal_hash.c
# SRCS += stm32f4xx_hal_hash_ex.c
# SRCS += stm32f4xx_hal_hcd.c
# SRCS += stm32f4xx_hal_i2c.c
# SRCS += stm32f4xx_hal_i2c_ex.c
# SRCS += stm32f4xx_hal_i2s.c
# SRCS += stm32f4xx_hal_i2s_ex.c
# SRCS += stm32f4xx_hal_irda.c
# SRCS += stm32f4xx_hal_iwdg.c
# SRCS += stm32f4xx_hal_ltdc.c
# SRCS += stm32f4xx_hal_msp_template.c
# SRCS += stm32f4xx_hal_nand.c
# SRCS += stm32f4xx_hal_nor.c
# SRCS += stm32f4xx_hal_pccard.c
SRCS += stm32f4xx_hal_pcd.c
# SRCS += stm32f4xx_hal_pcd_ex.c
# SRCS += stm32f4xx_hal_pwr.c
# SRCS += stm32f4xx_hal_pwr_ex.c
# SRCS += stm32f4xx_hal_qspi.c
# SRCS += stm32f4xx_hal_rcc_ex.c
# SRCS += stm32f4xx_hal_rng.c
# SRCS += stm32f4xx_hal_rtc.c
# SRCS += stm32f4xx_hal_rtc_ex.c
# SRCS += stm32f4xx_hal_sai.c
# SRCS += stm32f4xx_hal_sai_ex.c
# SRCS += stm32f4xx_hal_sd.c
# SRCS += stm32f4xx_hal_sdram.c
# SRCS += stm32f4xx_hal_smartcard.c
# SRCS += stm32f4xx_hal_spdifrx.c
# SRCS += stm32f4xx_hal_spi.c
# SRCS += stm32f4xx_hal_sram.c
SRCS += stm32f4xx_hal_tim.c
# SRCS += stm32f4xx_hal_tim_ex.c
# SRCS += stm32f4xx_hal_uart.c
# SRCS += stm32f4xx_hal_usart.c
# SRCS += stm32f4xx_hal_wwdg.c
# SRCS += stm32f4xx_ll_fmc.c
# SRCS += stm32f4xx_ll_fsmc.c
# SRCS += stm32f4xx_ll_sdmmc.c
# SRCS += stm32f4xx_ll_usb.c


SRCS += stm32f4xx_ll_usb.c
SRCS += stm32f4xx_hal_pcd_ex.c

# FreeRTOS: rtos/Source -> /usr/share/FreeRTOS/Source
#SRCS += croutine.c
SRCS += list.c
SRCS += queue.c
SRCS += tasks.c
#SRCS += timers.c
# symlink to Source/portable/GCC/ARM_CM3/port.c <<< change
SRCS += heap_2.c
# symlink to Source/portable/GCC/ARM_CM4F/port.c <<< change
SRCS += port.c
# Beware: portmacro.h is symlink to
#   rtos/Source/portable/GCC/ARM_CM3/portmacro.h or CM4F

include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

