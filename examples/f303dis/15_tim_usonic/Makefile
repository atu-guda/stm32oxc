PROJ_NAME=tim8_usonic

USE_OXC = y
# USE_OXC_DEVIO = y
USE_OXC_DEBUG = y
USE_OXC_CONSOLE_USART = y
USE_OXC_CONSOLE_USB_CDC = n
USE_FREERTOS = y
# FREERTOS_HEAP = heap_2.c
# # default: heap_3.c
# USE_USB_DEFAULT_CDC = y

# TODO: copy to main location
OXCDIR = /home/atu/proj/stm32/oxc

SRCS  = main.cpp

#SRCS += oxc_i2c.cpp

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
# SRCS += stm32f3xx_hal_i2c.c
# SRCS += stm32f3xx_hal_i2c_ex.c
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
SRCS += stm32f3xx_hal_tim.c
SRCS += stm32f3xx_hal_tim_ex.c
# SRCS += stm32f3xx_hal_tsc.c
SRCS += stm32f3xx_hal_uart.c
# SRCS += stm32f3xx_hal_uart_ex.c
# SRCS += stm32f3xx_hal_usart.c
# SRCS += stm32f3xx_hal_wwdg.c
# SRCS += stm32f3xx_ll_fmc.c



include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

