PROJ_NAME=uart_console


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
# SRCS += stm32f4xx_hal_pcd.c
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
# SRCS += stm32f4xx_hal_tim.c
# SRCS += stm32f4xx_hal_tim_ex.c
SRCS += stm32f4xx_hal_uart.c
# SRCS += stm32f4xx_hal_usart.c
# SRCS += stm32f4xx_hal_wwdg.c
# SRCS += stm32f4xx_ll_fmc.c
# SRCS += stm32f4xx_ll_fsmc.c
# SRCS += stm32f4xx_ll_sdmmc.c
# SRCS += stm32f4xx_ll_usb.c


include ../common/board_add.mk
include $(OXCDIR)/mk/common_cortex.mk
# ../../../mk/common_cortex.mk

include $(wildcard $(DEPSDIR)/*.d)
#

