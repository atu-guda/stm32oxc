PROJ_NAME=ssd1306
MCTYPE = STM32F303

MCINCTYPE = $(MCTYPE)xx

LINK=$(CXX)

USE_OXC = y
USE_FREERTOS = y
NO_STDLIB = y
# USE_USB_DEFAULT_CDC = y
USE_FONTS = y

# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc

CFLAGS_ADD := -D$(MCINCTYPE) -DSTM32F303xC -DHSE_VALUE=8000000
CFLAGS_ADD += -Wno-unused-parameter
#
LDSCRIPT = $(STMLD)/STM32F303VC_FLASH.ld

SRCS  = main.cpp
SRCS += stm32f3xx_hal_msp.cpp
SRCS += system_stm32f3xx.c
SRCS += startup_stm32f303xc.s

SRCS += stm32f3_clock_HSE072b.c

SRCS += oxc_base.cpp
SRCS += oxc_gpio.cpp
SRCS += oxc_devio.cpp
SRCS += oxc_rtosqueue.cpp
SRCS += oxc_osfun.cpp
SRCS += oxc_console.cpp
SRCS += oxc_debug1.cpp
SRCS += oxc_debug_i2c.cpp
SRCS += oxc_usartio.cpp
# SRCS += oxc_usbcdcio.cpp
SRCS += oxc_smallrl.cpp
# SRCS += oxc_smallrl_q.cpp
SRCS += oxc_common1.cpp
#SRCS += oxc_i2c.cpp
SRCS += oxc_pixbuf.cpp
SRCS += oxc_pixbuf1v.cpp
SRCS += oxc_ssd1306.cpp


# stm32cube sources.
# Base:
SRCS += stm32f3xx_hal.c
SRCS += stm32f3xx_hal_cortex.c
SRCS += stm32f3xx_hal_gpio.c
SRCS += stm32f3xx_hal_rcc.c

# aux:
# SRCS += stm32f3xx_hal_adc.c
# SRCS += stm32f3xx_hal_adc_ex.c
# SRCS += stm32f3xx_hal_can.c
# SRCS += stm32f3xx_hal_cec.c
# SRCS += stm32f3xx_hal_comp.c
# SRCS += stm32f3xx_hal_crc.c
# SRCS += stm32f3xx_hal_crc_ex.c
# SRCS += stm32f3xx_hal_dac.c
# SRCS += stm32f3xx_hal_dac_ex.c
# SRCS += stm32f3xx_hal_dma.c
# SRCS += stm32f3xx_hal_flash.c
# SRCS += stm32f3xx_hal_flash_ex.c
# SRCS += stm32f3xx_hal_hrtim.c
SRCS += stm32f3xx_hal_i2c.c
SRCS += stm32f3xx_hal_i2c_ex.c
# SRCS += stm32f3xx_hal_i2s.c
# SRCS += stm32f3xx_hal_i2s_ex.c
# SRCS += stm32f3xx_hal_irda.c
# SRCS += stm32f3xx_hal_iwdg.c
# SRCS += stm32f3xx_hal_msp_template.c
# SRCS += stm32f3xx_hal_nand.c
# SRCS += stm32f3xx_hal_nor.c
# SRCS += stm32f3xx_hal_opamp.c
# SRCS += stm32f3xx_hal_opamp_ex.c
# SRCS += stm32f3xx_hal_pccard.c
# SRCS += stm32f3xx_hal_pcd.c
# SRCS += stm32f3xx_hal_pcd_ex.c
# SRCS += stm32f3xx_hal_ppp.c
# SRCS += stm32f3xx_hal_pwr.c
# SRCS += stm32f3xx_hal_pwr_ex.c
SRCS += stm32f3xx_hal_rcc_ex.c
# SRCS += stm32f3xx_hal_rtc.c
# SRCS += stm32f3xx_hal_rtc_ex.c
# SRCS += stm32f3xx_hal_sdadc.c
# SRCS += stm32f3xx_hal_smartcard.c
# SRCS += stm32f3xx_hal_smartcard_ex.c
# SRCS += stm32f3xx_hal_smbus.c
# SRCS += stm32f3xx_hal_spi.c
# SRCS += stm32f3xx_hal_sram.c
# SRCS += stm32f3xx_hal_tim.c
# SRCS += stm32f3xx_hal_tim_ex.c
# SRCS += stm32f3xx_hal_tsc.c
SRCS += stm32f3xx_hal_uart.c
# SRCS += stm32f3xx_hal_uart_ex.c
# SRCS += stm32f3xx_hal_usart.c
# SRCS += stm32f3xx_hal_wwdg.c
# SRCS += stm32f3xx_ll_fmc.c

SRCS += font8.c
SRCS += font12.c
SRCS += font16.c
SRCS += font20.c
SRCS += font24.c


# FreeRTOS: rtos/Source -> /usr/share/FreeRTOS/Source
#SRCS += croutine.c
SRCS += list.c
SRCS += queue.c
SRCS += tasks.c
#SRCS += timers.c
# symlink to Source/portable/GCC/ARM_CM3/port.c <<< change
#SRCS += heap_2.c
SRCS += heap_3.c
# symlink to Source/portable/GCC/ARM_CM4F/port.c <<< change
SRCS += port.c
# Beware: portmacro.h is symlink to
#   rtos/Source/portable/GCC/ARM_CM3/portmacro.h or CM4F

include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

